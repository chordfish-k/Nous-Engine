#pragma once

#include "AssetMetadata.h"

namespace Nous
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
	};
}

