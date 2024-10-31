#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Scene/Scene.h"

namespace Nous
{
	class SceneImporter
	{
	public:
		// AssetMetadata��filepath���������Ŀָ����asset�ļ��е�
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);

		// ֱ�Ӵ�filesystem�ж�ȡ�ļ���·������ڹ���Ŀ¼
		static Ref<Scene> LoadScene(const std::filesystem::path& path);

		static void SaveScene(Ref<Scene> scene, const std::filesystem::path& path);
	};
}

