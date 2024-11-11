#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"

#include <map>

namespace Nous
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;

		AssetHandle ImportAsset(const std::filesystem::path& filepath);
		void RemoveAsset(AssetHandle handle);
		void ReloadAsset(AssetHandle handle);

		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;

		AssetRegistry& GetAssetRegistry() { return m_AssetRegistry; }

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

		static void SerializeAssetRegistry(AssetRegistry& reg, const std::filesystem::path& filepath);
		static bool DeserializeAssetRegistry(AssetRegistry& reg, const std::filesystem::path& filepath);

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}
