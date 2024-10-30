#pragma once

#include <glm/glm.hpp>

namespace Nous {

    class Camera
    {
    public:
        enum class ProjectionType { Perspective = 0, Orthographic = 1 };
    public:
        Camera() = default;
        Camera(const glm::mat4& projection)
            : m_Projection(projection) {};

        virtual ~Camera() = default;

        const glm::mat4& GetProjectionMatrix() const { return m_Projection; }

    protected:
        glm::mat4 m_Projection = glm::mat4(1.0f);
    };

}

