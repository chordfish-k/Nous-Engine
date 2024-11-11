#pragma once

#include "AssetManagerBase.h"

#include "Nous/Project/Project.h"

namespace Nous
{
	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetHandleValid(handle);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);
		}

		static std::string GetAssetFileName(AssetHandle handle)
		{
			std::string btnLabel = "None";
			if (handle != 0)
			{
				if (AssetManager::IsAssetHandleValid(handle))
				{
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(handle);
					btnLabel = metadata.FilePath.filename().string();
				}
				else
				{
					btnLabel = "Invalid";
				}
			}
			return btnLabel;
		}
	};
}
