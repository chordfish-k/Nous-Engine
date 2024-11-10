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
        case Nous::AssetType::Prefab:        return "AssetType::Prefab";
        case Nous::AssetType::Texture2D:    return "AssetType::Texture2D";
        case Nous::AssetType::AnimClip:    return "AssetType::AnimClip";
        case Nous::AssetType::AnimMachine:    return "AssetType::AnimMachine";
        }
        return "AssetType::<Invalid>";
    }

    AssetType AssetTypeFromString(std::string_view assetType)
    {

        if (assetType == "AssetType::None")         return Nous::AssetType::None;
        if (assetType == "AssetType::Scene")        return Nous::AssetType::Scene;
        if (assetType == "AssetType::Prefab")        return Nous::AssetType::Prefab;
        if (assetType == "AssetType::Texture2D")    return Nous::AssetType::Texture2D;
        if (assetType == "AssetType::AnimClip")    return Nous::AssetType::AnimClip;
        if (assetType == "AssetType::AnimMachine")    return Nous::AssetType::AnimMachine;
        
        return AssetType::None;
    }
}