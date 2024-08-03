#pragma once

#include <glm/glm.hpp>

namespace Nous {

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
}
