#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class SceneImporter
	{
	public:
		// AssetMetadata的filepath是相对于项目指定的asset文件夹的
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);

		// 直接从filesystem中读取文件，路径相对于工作目录
		static Ref<Scene> LoadScene(const std::filesystem::path& path);

		static void SaveScene(Ref<Scene> scene, const std::filesystem::path& path);
	};
}

