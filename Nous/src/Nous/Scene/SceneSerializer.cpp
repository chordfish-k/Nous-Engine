#include "pch.h"
#include "SceneSerializer.h"

#include "Nous/Scene/Entity.h"
#include "Nous/Scene/Component.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
namespace YAML {

    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };


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

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

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

    static std::string RigidBody2DBodyTypeToString(CRigidbody2D::BodyType bodyType)
    {
        switch (bodyType)
        {
            case CRigidbody2D::BodyType::Static:    return "Static";
            case CRigidbody2D::BodyType::Dynamic:   return "Dynamic";
            case CRigidbody2D::BodyType::Kinematic: return "Kinematic";
        }

        NS_CORE_ASSERT(false, "未知的 body type");
        return {};
    }

    static CRigidbody2D::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")    return CRigidbody2D::BodyType::Static;
        if (bodyTypeString == "Dynamic")   return CRigidbody2D::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic") return CRigidbody2D::BodyType::Kinematic;

        NS_CORE_ASSERT(false, "未知的 body type");
        return CRigidbody2D::BodyType::Static;
    }

    SceneSerializer::SceneSerializer(const Ref <Scene>& scene)
        : m_Scene(scene)
    {
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        // 必须具有ID组件
        NS_CORE_ASSERT(entity.HasComponent<CUuid>());

        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

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

        if (entity.HasComponent<CCircleRenderer>())
        {
            out << YAML::Key << "CCircleRenderer";
            out << YAML::BeginMap; // CircleRendererComponent

            auto& circleRendererComponent = entity.GetComponent<CCircleRenderer>();
            out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
            out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

            out << YAML::EndMap; // CircleRendererComponent
        }

        if (entity.HasComponent<CRigidbody2D>())
        {
            out << YAML::Key << "CRigidbody2D";
            out << YAML::BeginMap; // CRigidbody2D

            auto& rb2dComponent = entity.GetComponent<CRigidbody2D>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

            out << YAML::EndMap; // CRigidbody2D
        }

        if (entity.HasComponent<CBoxCollider2D>())
        {
            out << YAML::Key << "CBoxCollider2D";
            out << YAML::BeginMap; // CBoxCollider2D

            auto& bc2dComponent = entity.GetComponent<CBoxCollider2D>();
            out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
            out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

            out << YAML::EndMap; // CBoxCollider2D
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

                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

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

                auto circleRendererComponent = entity["CCircleRenderer"];
                if (circleRendererComponent)
                {
                    auto& crc = deserializedEntity.AddComponent<CCircleRenderer>();
                    crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    crc.Thickness = circleRendererComponent["Thickness"].as<float>();
                    crc.Fade = circleRendererComponent["Fade"].as<float>();
                }

                auto rigidbody2DComponent = entity["CRigidbody2D"];
                if (rigidbody2DComponent)
                {
                    auto& rb2d = deserializedEntity.AddComponent<CRigidbody2D>();
                    rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
                    rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
                }

                auto boxCollider2DComponent = entity["CBoxCollider2D"];
                if (boxCollider2DComponent)
                {
                    auto& bc2d = deserializedEntity.AddComponent<CBoxCollider2D>();
                    bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    bc2d.Density = boxCollider2DComponent["Density"].as<float>();
                    bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
                    bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
                    bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
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