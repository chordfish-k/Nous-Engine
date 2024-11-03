#include "pch.h"
#include "SceneSerializer.h"

#include "Nous/Scene/Entity.h"
#include "Nous/Scene/Component.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Core/UUID.h"
#include "Nous/Project/Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

// TODO：如果发布游戏，应该将其打包成二进制格式读取
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

    template<>
    struct convert<Nous::UUID>
    {
        static Node encode(const Nous::UUID& uuid)
        {
            Node node;
            node.push_back((uint64_t)uuid);
            return node;
        }

        static bool decode(const Node& node, Nous::UUID& uuid)
        {
            uuid = node.as<uint64_t>();
            return true;
        }
    };
}

namespace Nous {

#define WRITE_SCRIPT_FIELD(FieldType, Type)             \
    case ScriptFieldType::FieldType:                    \
        out << scriptField.GetValue<Type>();            \
        break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}

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

        if (entity.HasComponent<CMonoScript>())
        {
            auto& scriptComponent = entity.GetComponent<CMonoScript>();

            out << YAML::Key << "CMonoScript";
            out << YAML::BeginMap; // MonoScript
            out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;
            
            // 字段 Fields
            Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
            const auto& fields = entityClass->GetFields();
            if (fields.size() > 0)
            {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                out << YAML::BeginSeq;
                for (const auto& [name, field] : fields)
                {
                    if (entityFields.find(name) == entityFields.end())
                        continue;

                    out << YAML::BeginMap; // ScriptField
                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

                    out << YAML::Key << "Data" << YAML::Value;
                    ScriptFieldInstance& scriptField = entityFields.at(name);

                    switch (field.Type)
                    {
                        WRITE_SCRIPT_FIELD(Float,   float       );
                        WRITE_SCRIPT_FIELD(Double,  double      );
                        WRITE_SCRIPT_FIELD(Bool,    bool        );
                        WRITE_SCRIPT_FIELD(Char,    char        );
                        WRITE_SCRIPT_FIELD(Byte,    int8_t      );
                        WRITE_SCRIPT_FIELD(Short,   int16_t     );
                        WRITE_SCRIPT_FIELD(Int,     int32_t     );
                        WRITE_SCRIPT_FIELD(Long,    int64_t     );
                        WRITE_SCRIPT_FIELD(UByte,   uint8_t     );
                        WRITE_SCRIPT_FIELD(UShort,  uint16_t    );
                        WRITE_SCRIPT_FIELD(UInt,    uint32_t    );
                        WRITE_SCRIPT_FIELD(ULong,   uint64_t    );
                        WRITE_SCRIPT_FIELD(Vector2, glm::vec2   );
                        WRITE_SCRIPT_FIELD(Vector3, glm::vec3   );
                        WRITE_SCRIPT_FIELD(Vector4, glm::vec4   );
                        WRITE_SCRIPT_FIELD(Entity,  UUID        );
                    }
                    out << YAML::EndMap; // ScriptField
                }
                out << YAML::EndSeq;
            }
            
            out << YAML::EndMap; // MonoScript
        }

        if (entity.HasComponent<CSpriteRenderer>())
        {
            out << YAML::Key << "CSpriteRenderer";
            out << YAML::BeginMap; // CSpriteRenderer

            auto& spriteRendererComponent = entity.GetComponent<CSpriteRenderer>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;

            out << YAML::Key << "TextureHandle" << YAML::Value << spriteRendererComponent.Texture;

            out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;

            out << YAML::Key << "SheetWidth" << YAML::Value << spriteRendererComponent.SheetWidth;
            out << YAML::Key << "SheetHeight" << YAML::Value << spriteRendererComponent.SheetHeight;
            out << YAML::Key << "Index" << YAML::Value << spriteRendererComponent.Index;

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

        if (entity.HasComponent<CCircleCollider2D>())
        {
            out << YAML::Key << "CCircleCollider2D";
            out << YAML::BeginMap; // CCircleCollider2D

            auto& cc2dComponent = entity.GetComponent<CCircleCollider2D>();
            out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
            out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
            out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

            out << YAML::EndMap; // CCircleCollider2D
        }

        if (entity.HasComponent<CTextRenderer>())
        {
            out << YAML::Key << "CTextRenderer";
            out << YAML::BeginMap; // CTextRenderer

            auto& textComponent = entity.GetComponent<CTextRenderer>();
            out << YAML::Key << "TextString" << YAML::Value << textComponent.TextString;
            // TODO: FontAsset
            out << YAML::Key << "Color" << YAML::Value << textComponent.Color;
            out << YAML::Key << "Kerning" << YAML::Value << textComponent.Kerning;
            out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;

            out << YAML::EndMap; // CTextRenderer
        }

        if (entity.HasComponent<CAnimPlayer>())
        {
            out << YAML::Key << "CAnimPlayer";
            out << YAML::BeginMap; // CAnimPlayer

            auto& textComponent = entity.GetComponent<CAnimPlayer>();
            out << YAML::Key << "AnimClip" << YAML::Value << textComponent.AnimClip;

            out << YAML::EndMap; // CAnimPlayer
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::filesystem::path& filepath)
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

    void SceneSerializer::SerializeRuntime(const std::filesystem::path& filepath)
    {
        // 未实现
        NS_CORE_ASSERT(false);
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (YAML::ParserException& e)
        {
            NS_CORE_ERROR("无法加载 .nous 文件 '{0}'\n     {1}", filepath, e.what());
            return false;
        }

        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        NS_CORE_TRACE("正在解析场景文件 '{0}'", sceneName);

        auto entities = data["Entities"];
        if (!entities)
            return false;

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

            auto scriptComponent = entity["CMonoScript"];
            if (scriptComponent)
            {
                auto& sc = deserializedEntity.AddComponent<CMonoScript>();
                sc.ClassName = scriptComponent["ClassName"].as<std::string>();
                
                auto scriptFields = scriptComponent["ScriptFields"];
                Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);

                if (scriptFields && entityClass)
                {
                    const auto& fields = entityClass->GetFields();
                    auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

                    for (auto scriptField : scriptFields)
                    {
                        std::string name = scriptField["Name"].as<std::string>();
                        std::string typeString = scriptField["Type"].as<std::string>();
                        ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

                        ScriptFieldInstance& fieldInstance = entityFields[name];

                        // NS_CORE_ASSERT(fields.find(name) != fields.end());
                        if (fields.find(name) == fields.end())
                            continue;

                        fieldInstance.Field = fields.at(name);

                        switch (type)
                        {
                            READ_SCRIPT_FIELD(Float,    float       );
                            READ_SCRIPT_FIELD(Double,   double      );
                            READ_SCRIPT_FIELD(Bool,     bool        );
                            READ_SCRIPT_FIELD(Char,     char        );
                            READ_SCRIPT_FIELD(Byte,     int8_t      );
                            READ_SCRIPT_FIELD(Short,    int16_t     );
                            READ_SCRIPT_FIELD(Int,      int32_t     );
                            READ_SCRIPT_FIELD(Long,     int64_t     );
                            READ_SCRIPT_FIELD(UByte,    uint8_t     );
                            READ_SCRIPT_FIELD(UShort,   uint16_t    );
                            READ_SCRIPT_FIELD(UInt,     uint32_t    );
                            READ_SCRIPT_FIELD(ULong,    uint64_t    );
                            READ_SCRIPT_FIELD(Vector2,  glm::vec2   );
                            READ_SCRIPT_FIELD(Vector3,  glm::vec3   );
                            READ_SCRIPT_FIELD(Vector4,  glm::vec4   );
                            READ_SCRIPT_FIELD(Entity,   UUID        );
                        }
                    }
                }
            }

            auto spriteRendererComponent = entity["CSpriteRenderer"];
            if (spriteRendererComponent)
            {
                auto& src = deserializedEntity.AddComponent<CSpriteRenderer>();
                src.Color = spriteRendererComponent["Color"].as<glm::vec4>();

                if (spriteRendererComponent["TexturePath"])
                {
                    /*std::string texturePath = spriteRendererComponent["TexturePath"].as<std::string>();
                    auto path = Project::GetAssetsFileSystemPath(texturePath);
                    src.Texture = Texture2D::Create(path);*/
                }

                if (spriteRendererComponent["TextureHandle"])
                    src.Texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();

                if (spriteRendererComponent["TilingFactor"])
                    src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

                if (spriteRendererComponent["SheetWidth"])
                    src.SheetWidth = spriteRendererComponent["SheetWidth"].as<int>();

                if (spriteRendererComponent["SheetHeight"])
                    src.SheetHeight = spriteRendererComponent["SheetHeight"].as<int>();

                if (spriteRendererComponent["Index"])
                    src.Index = spriteRendererComponent["Index"].as<int>();
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

            auto circleCollider2D = entity["CCircleCollider2D"];
            if (circleCollider2D)
            {
                auto& cc2d = deserializedEntity.AddComponent<CCircleCollider2D>();
                cc2d.Offset = circleCollider2D["Offset"].as<glm::vec2>();
                cc2d.Radius = circleCollider2D["Radius"].as<float>();
                cc2d.Density = circleCollider2D["Density"].as<float>();
                cc2d.Friction = circleCollider2D["Friction"].as<float>();
                cc2d.Restitution = circleCollider2D["Restitution"].as<float>();
                cc2d.RestitutionThreshold = circleCollider2D["RestitutionThreshold"].as<float>();
            }

            auto textRenderer = entity["CTextRenderer"];
            if (textRenderer)
            {
                auto& tc = deserializedEntity.AddComponent<CTextRenderer>();
                tc.TextString = textRenderer["TextString"].as<std::string>();
                tc.Color = textRenderer["Color"].as<glm::vec4>();
                tc.Kerning = textRenderer["Kerning"].as<float>();
                tc.LineSpacing = textRenderer["LineSpacing"].as<float>();
            }

            auto animPlayer = entity["CAnimPlayer"];
            if (animPlayer)
            {
                auto& ap = deserializedEntity.AddComponent<CAnimPlayer>();
                ap.AnimClip = animPlayer["AnimClip"].as<AssetHandle>();
            }
        }
        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::filesystem::path& filepath)
    {
        // 未实现
        NS_CORE_ASSERT(false);
        return false;
    }
}