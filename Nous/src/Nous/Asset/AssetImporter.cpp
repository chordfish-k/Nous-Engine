#include "pch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "SceneImporter.h"
#include "AnimClipImporter.h"
#include "AnimMachineImporter.h"

#include <map>

namespace Nous
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	// 装载不同资源类型的导入器
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D },
		{ AssetType::Scene, SceneImporter::ImportScene },
		{ AssetType::AnimClip, AnimClipImporter::ImportAnimClip },
		{ AssetType::AnimMachine, AnimMachineImporter::ImportAnimMachine }
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			NS_CORE_ERROR("没有合适的导入器，资源类型: {}", (uint16_t)metadata.Type);
			return nullptr;
		}
		return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
	}
}