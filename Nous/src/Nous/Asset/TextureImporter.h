#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Renderer/Texture.h"

namespace Nous
{
	class TextureImporter
	{
	public:
		// AssetMetadata的filepath是相对于项目指定的asset文件夹的
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		
		// 直接从filesystem中读取文件，路径相对于工作目录
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path);
	};
}

