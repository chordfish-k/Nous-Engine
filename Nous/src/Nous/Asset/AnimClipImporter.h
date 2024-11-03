#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Anim/AnimClip.h"

namespace Nous
{
	class AnimClipImporter
	{
	public:
		// AssetMetadata��filepath���������Ŀָ����asset�ļ��е�
		static Ref<AnimClip> ImportAnimClip(AssetHandle handle, const AssetMetadata& metadata);

		// ֱ�Ӵ�filesystem�ж�ȡ�ļ���·������ڹ���Ŀ¼
		static Ref<AnimClip> LoadAnimClip(const std::filesystem::path& path);

		static void SaveAnimClip(Ref<AnimClip> scene, const std::filesystem::path& path);
	};
}


