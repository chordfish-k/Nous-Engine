#include "pch.h"
#include "AssetManager.h"

#include "AssetImporter.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Nous
{
    static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
        { ".nous", AssetType::Scene},
        { ".nsprefab", AssetType::Prefab},
        { ".png", AssetType::Texture2D},
        { ".jpg", AssetType::Texture2D},
        { ".jpeg", AssetType::Texture2D},
        { ".nsani", AssetType::AnimClip},
        { ".nsanm", AssetType::AnimMachine},
    };

    static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
    {
        if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
        {
            NS_CORE_WARN("δ֪��׺����Դ����: {}", extension);
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
        // ������Ƿ���Ч
        if (!IsAssetHandleValid(handle))
            return nullptr;

        // �����Դ�Ƿ�δ���أ�����������
        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {
            asset = m_LoadedAssets.at(handle);
        }
        else
        {
            // ������Դ
            const AssetMetadata& metadata = GetMetadata(handle);
            asset = AssetImporter::ImportAsset(handle, metadata);
            if (!asset)
            {
                // ����ʧ��
                NS_CORE_ERROR("EditorAssetManager::GetAsset - ��Դ����ʧ��");
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

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
    {
        AssetHandle handle;
        AssetMetadata metadata;
        metadata.FilePath = filepath;
        metadata.Type = GetAssetTypeFromFileExtension(filepath.extension());
        if (metadata.Type == AssetType::None)
            return 0;

        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
        asset->Handle = handle;
        if (asset)
        {
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;
            SerializeAssetRegistry();
            return handle;
        }
        return 0;
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
        EditorAssetManager::SerializeAssetRegistry(m_AssetRegistry, path);
    }

    bool EditorAssetManager::DeserializeAssetRegistry()
    {
        auto path = Project::GetActiveAssetRegistryPath();
        return EditorAssetManager::DeserializeAssetRegistry(m_AssetRegistry, path);
    }

    void EditorAssetManager::SerializeAssetRegistry(AssetRegistry& reg, const std::filesystem::path& filepath)
    {
        YAML::Emitter out;
        {
            out << YAML::BeginMap; // Root
            out << YAML::Key << "AssetRegistry" << YAML::Value;

            out << YAML::BeginSeq;
            for (auto& [handle, metadata] : reg)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;
                std::string filepathStr = metadata.FilePath.generic_string(); // ��б��ת��б��
                out << YAML::Key << "FilePath" << YAML::Value << filepathStr;
                out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap; // Root
        }

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    bool EditorAssetManager::DeserializeAssetRegistry(AssetRegistry& reg, const std::filesystem::path& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (const std::exception& e)
        {
            NS_CORE_ERROR("�޷�������Դע��� '{0}'\n  {1}", filepath, e.what());
            return false;
        }

        auto rootNode = data["AssetRegistry"];
        if (!rootNode)
            return false;

        for (const auto& node : rootNode)
        {
            std::string relativePath = node["FilePath"].as<std::string>();
            std::filesystem::path path = Project::GetActiveAssetDirectory() / relativePath;
            if (!std::filesystem::exists(path))
                continue;
            AssetHandle handle = node["Handle"].as<uint64_t>();
            auto& metadata = reg[handle];
            metadata.FilePath = relativePath;
            metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
        }

        return true;
    }
}