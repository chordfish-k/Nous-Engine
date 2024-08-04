#include "pch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>


namespace Nous {

    SceneCamera::SceneCamera()
    {
        RecalculateProjection();
    }

    void SceneCamera::SetOrtho(float size, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;

        m_OrthoSize = size;
        m_OrthoNear = nearClip;
        m_OrthoFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetPersp(float FOV, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Perspective;

        m_PerspFOV = FOV;
        m_PerspNear = nearClip;
        m_PerspFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        m_AspectRatio = (float) width / (float) height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_Projection = glm::perspective(m_PerspFOV, m_AspectRatio, m_PerspNear, m_PerspFar);
        }
        else
        {
            float orthoLeft = -m_OrthoSize * m_AspectRatio * 0.5f;
            float orthoRight = m_OrthoSize * m_AspectRatio * 0.5f;
            float orthoTop = -m_OrthoSize * 0.5f;
            float orthoBottom = m_OrthoSize * 0.5f;

            m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthoNear, m_OrthoFar);
        }
    }
}
