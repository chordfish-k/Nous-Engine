﻿#pragma once

#include "Nous/Scene/SceneCamera.h"
#include "Nous/Renderer/Texture.h"
#include "Nous/Renderer/Font.h"
#include "Nous/Core/UUID.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Nous {

    struct CUuid
    {
        UUID ID;
        CUuid() = default;
        CUuid(const CUuid&) = default;
    };

    struct CTag
    {
        std::string Tag;
        CTag() = default;
        CTag(const CTag&) = default;
        CTag(const std::string& tag)
            : Tag(tag) {}
    };

    struct CTransform
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
        glm::mat4 ParentTransform{ 1.0f };
        UUID Parent = 0;
        bool Open = true;
        std::vector<UUID> Children;

        CTransform() = default;
        CTransform(const CTransform&) = default;
        CTransform(const glm::vec3& translation)
            : Translation(translation) {};

        glm::mat4 GetTransform() const
        {
            return glm::translate(glm::mat4(1.0f), Translation)
                   * glm::toMat4(glm::quat(Rotation))
                   * glm::scale(glm::mat4(1.0f), Scale);
        }

        operator glm::mat4() const { return GetTransform(); }
    };

    struct CSpriteRenderer
    {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        AssetHandle Texture = 0;
        float TilingFactor = 1.0f;
        int SheetWidth = 0; // 0 = Max
        int SheetHeight = 0;
        int Index = 0;

        CSpriteRenderer() = default;
        CSpriteRenderer(const CSpriteRenderer&) = default;
        CSpriteRenderer(const glm::vec4& color)
            : Color(color) {};

        operator glm::vec4&() { return Color; }
        operator const glm::vec4&() const { return Color; }
    };

    struct CAnimPlayer
    {
        AssetHandle AnimClip = 0;
        AssetType Type = AssetType::AnimClip;

        CAnimPlayer() = default;
        CAnimPlayer(const CAnimPlayer&) = default;
    };

    struct CCircleRenderer
    {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        float Thickness = 1.0f;
        float Fade = 0.005f;

        CCircleRenderer() = default;
        CCircleRenderer(const CCircleRenderer&) = default;
    };

    struct CTextRenderer
    {
        std::string TextString;
        Ref<Font> FontAsset = Font::GetDefault();
        glm::vec4 Color{ 1.0f };
        float Kerning = 0.0f; // 字距
        float LineSpacing = 0.0f; // 行距
    };

    struct CCamera
    {
        SceneCamera Camera;
        bool Primary = true; // 是否为主摄像机
        bool FixedAspectRatio = false; // 是否需要固定视窗高宽比

        CCamera() = default;
        CCamera(const CCamera&) = default;
    };

    struct CMonoScript
    {
        std::string ClassName;

        CMonoScript() = default;
        CMonoScript(const CMonoScript&) = default;
    };

    // 预先声明
    class ScriptableEntity;

    struct CNativeScript
    {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity* (*InitScript)();
        void (*DestroyScript)(CNativeScript*);

        template<typename T>
        void Bind()
        {
            InitScript = []() { return static_cast<ScriptableEntity*>(new T());};
            DestroyScript = [](CNativeScript* script) { delete script->Instance; script->Instance = nullptr;};
        }
    };

    struct CRigidbody2D
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        // 存放运行时的物理Body
        void* RuntimeBody = nullptr;

        CRigidbody2D() = default;
        CRigidbody2D(const CRigidbody2D&) = default;
    };

    struct CBoxCollider2D
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 0.5f, 0.5f };

        // TODO 可能要移动到 物理材质系统
        float Density = 1.0f; // 密度
        float Friction = 0.5f; // 摩擦力
        float Restitution = 0.0f; // 恢复系数 | 弹性系数
        float RestitutionThreshold = 0.5f; // 弹性阈值

        // 存放运行时的Fixture
        void* RuntimeFixture = nullptr;

        CBoxCollider2D() = default;
        CBoxCollider2D(const CBoxCollider2D&) = default;
    };

    struct CCircleCollider2D
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        float Radius = 0.5f;

        // TODO 可能要移动到 物理材质系统
        float Density = 1.0f; // 密度
        float Friction = 0.5f; // 摩擦力
        float Restitution = 0.0f; // 恢复系数 | 弹性系数
        float RestitutionThreshold = 0.5f; // 弹性阈值

        // 存放运行时的Fixture
        void* RuntimeFixture = nullptr;

        CCircleCollider2D() = default;
        CCircleCollider2D(const CCircleCollider2D&) = default;
    };


    template<typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
        ComponentGroup<CTransform, CSpriteRenderer, CCircleRenderer, 
        CCamera, CNativeScript, CMonoScript, CRigidbody2D, 
        CBoxCollider2D, CCircleCollider2D, CTextRenderer,
        CAnimPlayer>;
}
