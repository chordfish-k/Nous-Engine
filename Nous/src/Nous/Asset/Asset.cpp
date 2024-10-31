#include "pch.h"
#include "Asset.h"

namespace Nous
{
    std::string_view AssetTypeToString(AssetType type)
    {
        switch (type)
        {
        case Nous::AssetType::None:         return "AssetType::None";
        case Nous::AssetType::Scene:        return "AssetType::Scene";
        case Nous::AssetType::Texture2D:    return "AssetType::Texture";
        }
        return "AssetType::<Invalid>";
    }

    AssetType AssetTypeFromString(std::string_view assetType)
    {

        if (assetType == "AssetType::None")         return Nous::AssetType::None;
        if (assetType == "AssetType::Scene")        return Nous::AssetType::Scene;
        if (assetType == "AssetType::Texture")    return Nous::AssetType::Texture2D;
        
        return AssetType::None;
    }
}