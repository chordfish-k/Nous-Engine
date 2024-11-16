#pragma once

#include "Nous/Core/Base.h"

#include "Nous/Asset/RuntimeAssetManager.h"
#include "Nous/Asset/EditorAssetManager.h"

namespace Nous
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		AssetHandle StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath; // 相对于 AssetDirectory
		std::filesystem::path ScriptModulePath;
	};

	class Project
	{
	public:
		const std::filesystem::path& GetProjectDirectory() { return m_ProjectDirectory; }
		const std::filesystem::path& GetProjectFilePath() { return m_ProjectFilePath; }
		std::filesystem::path GetAssetDirectory() { return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory; }
		std::filesystem::path GetAssetRegistryPath() { return GetAssetDirectory() / s_ActiveProject->m_Config.AssetRegistryPath; }
		// 将来移动到资产管理器
		std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path) { return GetAssetDirectory() / path; }

		std::filesystem::path GetAssetAbsolutePath(const std::filesystem::path& path) { return GetAssetDirectory() / path; }

		static const std::filesystem::path& GetActiveProjectDirectory()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetProjectDirectory();
		}

		static const std::filesystem::path& GetActiveProjectFilePath()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetProjectFilePath();
		}

		static std::filesystem::path GetActiveAssetDirectory()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetDirectory();
		}

		static std::filesystem::path GetActiveAssetRegistryPath()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetRegistryPath();
		}

		// 将来移动到资产管理器
		static std::filesystem::path GetActiveAssetFileSystemPath(const std::filesystem::path& path)
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetFileSystemPath(path);
		}


		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); };
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); };

		static Ref<Project> New();
		static Ref<Project> New(const std::filesystem::path& path);
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
		static bool SaveActive();
		static bool SaveNew(Ref<Project> project);

		static void ExportProject();

	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		std::filesystem::path m_ProjectFilePath;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}
