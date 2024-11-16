#include "pch.h"
#include "Project.h"

#include "ProjectSerializer.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Asset/AssetManager.h"

#include <fstream>

namespace Nous
{
	

	Ref<Project> Project::New()
	{
		return CreateRef<Project>();
	}

	Ref<Project> Project::New(const std::filesystem::path& path)
	{
		auto proj = CreateRef<Project>();
		auto& config = proj->GetConfig();
		config.Name = path.filename().string();
		config.AssetRegistryPath = "AssetRegistry.nsasset";
		config.ScriptModulePath = std::filesystem::path("Bin\\App") / (config.Name + ".dll");
		config.AssetDirectory = ".";
		config.StartScene = 0;

		Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
		proj->m_AssetManager = editorAssetManager;
		proj->m_ProjectDirectory = path;
		proj->m_ProjectFilePath = path / (config.Name + ".nsproj");

		return proj;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();
		
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			project->m_ProjectFilePath = path;
			s_ActiveProject = project;
			// 载入资源管理器的数据
			Ref<EditorAssetManager> editorAssetManager = CreateRef<EditorAssetManager>();
			s_ActiveProject->m_AssetManager = editorAssetManager;
			editorAssetManager->DeserializeAssetRegistry();
			return s_ActiveProject;
		}
		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool Project::SaveActive()
	{
		SaveActive(s_ActiveProject->m_ProjectFilePath);
		return false;
	}

	bool Project::SaveNew(Ref<Project> project)
	{
		ProjectSerializer serializer(project);
		if (serializer.Serialize(project->m_ProjectFilePath))
		{
			EditorAssetManager::SerializeAssetRegistry(
				project->GetEditorAssetManager()->GetAssetRegistry(), 
				project->m_ProjectDirectory / project->GetConfig().AssetRegistryPath);

			return true;
		}
		return false;
	}

	void Project::ExportProject()
	{
		namespace fs = std::filesystem;

		ScriptEngine::InitApp();
		auto deps = ScriptEngine::FindDependencies();

		auto projDir = Project::GetActiveProjectDirectory();

		// 1.导出的文件在 项目目录/Build 文件夹，先新建
		auto dir = projDir / "Build" / Project::GetActive()->GetConfig().Name;

		if (fs::exists(dir))
			fs::remove_all(dir);
		fs::create_directories(dir);

		auto tempDir = fs::current_path().parent_path() / "template";
		if (!fs::exists(tempDir))
		{
			NS_CORE_ERROR("编辑器缺少必要文件(template文件夹)！");
			return;
		}

		// 2.复制项目资源：
		for (auto& dir_entry : fs::recursive_directory_iterator(tempDir)) {
			const auto& path = dir_entry.path();
			auto relative_path = path.lexically_relative(tempDir);
			fs::path dest_path = dir / relative_path;

			if (fs::is_directory(path)) {
				fs::create_directories(dest_path);
			}
			else if (fs::is_regular_file(path)) {
				fs::copy(path, dest_path, fs::copy_options::update_existing);
			}
		}

		auto gameDir = dir / "game";
		if (!fs::exists(gameDir))
			fs::create_directories(gameDir);

		// 2.1.复制资源
		auto regPath = Project::GetActive()->GetAssetRegistryPath();
		fs::copy_file(regPath, gameDir / fs::relative(regPath, projDir));

		auto projFilePath = Project::GetActive()->GetProjectFilePath();
		fs::copy_file(projFilePath, gameDir / fs::relative(projFilePath.parent_path() / "game.nsproj", projDir));

		const AssetRegistry& reg = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (auto& [_, metadata] : reg)
		{
			auto path = Project::GetActive()->GetAssetFileSystemPath(metadata.FilePath);
			path = std::filesystem::absolute(path);
			if (std::filesystem::exists(path))
			{
				auto to = std::filesystem::absolute(gameDir / (std::filesystem::relative(path, projDir)));

				if (!std::filesystem::exists(to))
				{
					if (!std::filesystem::exists(to.parent_path()))
						std::filesystem::create_directories(to.parent_path());
					std::filesystem::copy_file(path, to);
				}
			}
		}

		// 复制C#程序集
		auto appAssemblyPath = Project::GetActiveProjectDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		if (fs::exists(appAssemblyPath))
		{
			auto to = gameDir / fs::relative(appAssemblyPath, projDir);
			fs::create_directories(to.parent_path());
			fs::copy_file(appAssemblyPath,  to);
		}

		
		// 3.复制依赖
		ScriptEngine::InitApp();
		auto dependencis = ScriptEngine::FindDependencies();
		const auto folders = fs::path("mono") / "lib" / "mono" / "4.5";
		if (!fs::exists(dir / folders))
			fs::create_directories(dir / folders);
		auto depPath = fs::current_path() / folders;
		for (auto& dep : dependencis)
		{
			dep += ".dll";
			auto depFile = depPath / dep;
			auto depTo = dir / folders / dep;
			if (fs::exists(depFile))
				fs::copy_file(depFile, depTo);
		}

	}

	
}