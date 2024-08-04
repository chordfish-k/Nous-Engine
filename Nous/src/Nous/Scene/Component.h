#pragma once

#include <glm/glm.hpp>

#include "Nous/Renderer/Camera.h"

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
        glm::mat4 Transform{ 1.0f };

        CTransform() = default;
        CTransform(const CTransform&) = default;
        CTransform(const glm::mat4& transform)
            : Transform(transform) {};

        operator glm::mat4&() { return Transform; }
        operator const glm::mat4&() const { return Transform; }
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
        Nous::Camera Camera;
        bool Primary = true; // 是否为主摄像机

        CCamera() = default;
        CCamera(const CCamera&) = default;
        CCamera(const glm::mat4& projection)
            : Camera(projection) {};
    };
}
