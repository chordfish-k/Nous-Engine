#include "pch.h"
#include "AssetManager.h"

#include "AssetImporter.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Nous
{
    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
        { ".nous", AssetType::Scene},
        { ".png", AssetType::Texture2D},
        { ".jpg", AssetType::Texture2D},
        { ".jpeg", AssetType::Texture2D},
    };

    static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
    {
        if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
        {
            NS_CORE_WARN("未知后缀的资源类型: {}", extension);
            return AssetType::None;
        }
        return s_AssetExtensionMap.at(extension);
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::string_view& v)
    {
        out << std::string(v.data(), v.size());
        return out;
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
    {
        // 检查句柄是否有效
        if (!IsAssetHandleValid(handle))
            return nullptr;

        // 检查资源是否未加载，如果是则加载
        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {
            asset = m_LoadedAssets.at(handle);
        }
        else
        {
            // 加载资源
            const AssetMetadata& metadata = GetMetadata(handle);
            asset = AssetImporter::ImportAsset(handle, metadata);
            if (!asset)
            {
                // 导入失败
                NS_CORE_ERROR("EditorAssetManager::GetAsset - 资源导入失败");
            }
            m_LoadedAssets[handle] = asset;
        }
        return asset;
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
    {
        return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
    {
        return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
    }

    AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
    {
        if (!IsAssetHandleValid(handle))
            return AssetType::None;

        return m_AssetRegistry.at(handle).Type;
    }

    void EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.FilePath = filepath;
        metadata.Type = GetAssetTypeFromFileExtension(filepath.extension());
        if (metadata.Type == AssetType::None)
            return;

        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
        asset->Handle = handle;
        if (asset)
        {
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;
            SerializeAssetRegistry();
        }
    }

    void EditorAssetManager::RemoveAsset(AssetHandle handle)
    {
        NS_CORE_ASSERT(m_AssetRegistry.find(handle) != m_AssetRegistry.end());
        m_AssetRegistry.erase(handle);

        if (m_LoadedAssets.find(handle) != m_LoadedAssets.end())
        {
            m_LoadedAssets[handle]->Handle = 0;
            m_LoadedAssets.erase(handle);
        }
        
        SerializeAssetRegistry();
    }

    void EditorAssetManager::ReloadAsset(AssetHandle handle)
    {
        if (m_LoadedAssets.find(handle) != m_LoadedAssets.end())
        {
            m_LoadedAssets[handle] = AssetImporter::ImportAsset(handle, m_AssetRegistry[handle]);
        }
    }

    const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
    {
        static AssetMetadata s_NullMetadata;
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
            return s_NullMetadata;

        return it->second;
    }

    const std::filesystem::path& EditorAssetManager::GetFilePath(AssetHandle handle) const
    {
        return GetMetadata(handle).FilePath;
    }

    void EditorAssetManager::SerializeAssetRegistry()
    {
        auto path = Project::GetActiveAssetRegistryPath();

        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "AssetRegistry" << YAML::Value;

            out << YAML::BeginSeq;
            for (const auto& [handle, metadata] : m_AssetRegistry)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;
                std::string filepathStr = metadata.FilePath.generic_string(); // 反斜杠转正斜杠
                out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
                out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(path);
        fout << out.c_str();
    }

    bool EditorAssetManager::DeserializeAssetRegistry()
    {
        auto path = Project::GetActiveAssetRegistryPath();
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string());
        }
        catch (const std::exception& e)
        {
            NS_CORE_ERROR("无法加载资源注册表 '{0}'\n  {1}", path, e.what());
            return false;
        }

        auto rootNode = data["AssetRegistry"];
        if (!rootNode)
            return false;

        for (const auto& node : rootNode)
        {
            AssetHandle handle = node["Handle"].as<uint64_t>();
            auto& metadata = m_AssetRegistry[handle];
            metadata.FilePath = node["FilePath"].as<std::string>();
            metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
        }

        return true;
    }
}