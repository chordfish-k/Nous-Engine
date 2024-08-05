#include "pch.h"
#include "SceneSerializer.h"

#include "Nous/Scene/Entity.h"
#include "Nous/Scene/Component.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
namespace YAML {

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}

namespace Nous {

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow; // FLow: 以[a, b, c]的形式输出
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref <Scene>& scene)
        : m_Scene(scene)
    {
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << "123456"; // TODO 改成 entity id

        if(entity.HasComponent<CTag>())
        {
            out << YAML::Key << "CTag";
            out << YAML::BeginMap;

            auto& tag = entity.GetComponent<CTag>();
            out << YAML::Key << "Tag" << YAML::Value << tag.Tag;

            out << YAML::EndMap;
        }

        if (entity.HasComponent<CTransform>())
        {
            out << YAML::Key << "CTransform";
            out << YAML::BeginMap; // CTransform

            auto& tc = entity.GetComponent<CTransform>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out << YAML::EndMap; // CTransform
        }

        if (entity.HasComponent<CCamera>())
        {
            out << YAML::Key << "CCamera";
            out << YAML::BeginMap; // CCamera

            auto& cameraComponent = entity.GetComponent<CCamera>();
            auto& camera = cameraComponent.Camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap; // Camera
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspFOV" << YAML::Value << camera.GetPerspFOV();
            out << YAML::Key << "PerspNear" << YAML::Value << camera.GetPerspNearClip();
            out << YAML::Key << "PerspFar" << YAML::Value << camera.GetPerspFarClip();
            out << YAML::Key << "OrthoSize" << YAML::Value << camera.GetOrthoSize();
            out << YAML::Key << "OrthoNear" << YAML::Value << camera.GetOrthoNearClip();
            out << YAML::Key << "OrthoFar" << YAML::Value << camera.GetOrthoFarClip();
            out << YAML::EndMap; // Camera

            out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

            out << YAML::EndMap; // CCamera
        }

        if (entity.HasComponent<CSpriteRenderer>())
        {
            out << YAML::Key << "CSpriteRenderer";
            out << YAML::BeginMap; // CSpriteRenderer

            auto& spriteRendererComponent = entity.GetComponent<CSpriteRenderer>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;

            out << YAML::EndMap; // CSpriteRenderer
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID){
            Entity entity = { entityID, m_Scene.get() };
            if (!entity)
               return;

            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        // 未实现
        NS_CORE_ASSERT(false);
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        YAML::Node data = YAML::LoadFile(filepath);
        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        NS_CORE_TRACE("正在解析场景文件 '{0}'", sceneName);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComponent = entity["CTag"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                NS_CORE_TRACE("解析实体 ID = {0}, name = {1}", uuid, name);

                Entity deserializedEntity = m_Scene->CreateEntity(name);

                auto transformComponent = entity["CTransform"];
                if (transformComponent)
                {
                    // 取出实体默认的Transform并赋值
                    auto& tc = deserializedEntity.GetComponent<CTransform>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CCamera"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CCamera>();

                    auto cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    cc.Camera.SetPerspFOV(cameraProps["PerspFOV"].as<float>());
                    cc.Camera.SetPerspNearClip(cameraProps["PerspNear"].as<float>());
                    cc.Camera.SetPerspFarClip(cameraProps["PerspFar"].as<float>());

                    cc.Camera.SetOrthoSize(cameraProps["OrthoSize"].as<float>());
                    cc.Camera.SetOrthoNearClip(cameraProps["OrthoNear"].as<float>());
                    cc.Camera.SetOrthoFarClip(cameraProps["OrthoFar"].as<float>());

                    cc.Primary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto spriteRendererComponent = entity["CSpriteRenderer"];
                if (spriteRendererComponent)
                {
                    auto& src = deserializedEntity.AddComponent<CSpriteRenderer>();
                    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                }
            }
        }

        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        // 未实现
        NS_CORE_ASSERT(false);
        return false;
    }
}