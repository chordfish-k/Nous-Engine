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
	// װ�ز�ͬ��Դ���͵ĵ�����
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
			NS_CORE_ERROR("û�к��ʵĵ���������Դ����: {}", (uint16_t)metadata.Type);
			return nullptr;
		}
		return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
	}
}