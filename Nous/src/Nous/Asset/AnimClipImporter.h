#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Anim/AnimClip.h"

namespace Nous
{
	class AnimClipImporter
	{
	public:
		// AssetMetadata的filepath是相对于项目指定的asset文件夹的
		static Ref<AnimClip> ImportAnimClip(AssetHandle handle, const AssetMetadata& metadata);

		// 直接从filesystem中读取文件，路径相对于工作目录
		static Ref<AnimClip> LoadAnimClip(const std::filesystem::path& path);

		static void SaveAnimClip(Ref<AnimClip> scene, const std::filesystem::path& path);
	};
}


