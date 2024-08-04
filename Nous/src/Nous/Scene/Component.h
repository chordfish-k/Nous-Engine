#pragma once

#include "Nous/Scene/SceneCamera.h"
#include "ScriptableEntity.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Nous {

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

        CTransform() = default;
        CTransform(const CTransform&) = default;
        CTransform(const glm::vec3& translation)
            : Translation(translation) {};

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
                                 * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
                                 * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

            return glm::translate(glm::mat4(1.0f), Translation)
                   * rotation
                   * glm::scale(glm::mat4(1.0f), Scale);
        }

        operator glm::mat4() const { return GetTransform(); }
    };

    struct CSpriteRenderer
    {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

        CSpriteRenderer() = default;
        CSpriteRenderer(const CSpriteRenderer&) = default;
        CSpriteRenderer(const glm::vec4& color)
            : Color(color) {};

        operator glm::vec4&() { return Color; }
        operator const glm::vec4&() const { return Color; }
    };

    struct CCamera
    {
        SceneCamera Camera;
        bool Primary = true; // 是否为主摄像机
        bool FixedAspectRatio = false; // 是否需要固定视窗高宽比

        CCamera() = default;
        CCamera(const CCamera&) = default;
    };

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
}
