#pragma once

#include "Nous/Core/UUID.h"

namespace Nous
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Texture2D,
		AnimClip,
		AnimMachine
	};

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view assetType);

	class Asset
	{
	public:
		AssetHandle Handle;	//×ÊÔ´¾ä±ú
		
		virtual AssetType GetType() const = 0;
	};
}

