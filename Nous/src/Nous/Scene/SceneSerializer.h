#pragma once

#include "Scene.h"

namespace Nous {

    class SceneSerializer
    {
    public:
        SceneSerializer(Scene* scene);
        SceneSerializer(const Ref<Scene>& scene);

        void Serialize(const std::filesystem::path& filepath);
        void SerializePrefabFrom(const std::filesystem::path& filepath, UUID root);
        void SerializeRuntime(const std::filesystem::path& filepath);

        bool Deserialize(const std::filesystem::path& filepath);
        bool DeserializePrefabTo(AssetHandle sceneHandle, UUID to, UUID* outRootUUID = nullptr);
        bool DeserializeRuntime(const std::filesystem::path& filepath);
    private:
        Scene* m_Scene = nullptr;
    };

}

