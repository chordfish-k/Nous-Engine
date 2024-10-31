#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Nous/Renderer/Texture.h"

namespace Nous
{
	class TextureImporter
	{
	public:
		// AssetMetadata��filepath���������Ŀָ����asset�ļ��е�
		static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
		
		// ֱ�Ӵ�filesystem�ж�ȡ�ļ���·������ڹ���Ŀ¼
		static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path);
	};
}

