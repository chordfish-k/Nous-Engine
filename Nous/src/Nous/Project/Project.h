#pragma once

#include "Nous/Core/Base.h"

#include "Nous/Asset/RuntimeAssetManager.h"
#include "Nous/Asset/EditorAssetManager.h"

namespace Nous
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath; // 相对于 AssetDirectory
		std::filesystem::path ScriptModulePath;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetsDirectory()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		static std::filesystem::path GetAssetsRegistryPath()
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return GetAssetsDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
		}

		// 将来移动到资产管理器
		static std::filesystem::path GetAssetsFileSystemPath(const std::filesystem::path& path)
		{
			NS_CORE_ASSERT(s_ActiveProject);
			return GetAssetsDirectory() / path;
		}
		
		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); };
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); };

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}
