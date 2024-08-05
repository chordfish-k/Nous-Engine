#pragma once

#include "Nous/Renderer/Camera.h"

namespace Nous {

    // 场景摄像机
    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType {Perspective = 0, Orthographic = 1};
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetOrtho(float size, float nearClip, float farClip);
        void SetPersp(float FOV, float nearClip, float farClip);

        void SetViewportSize(uint32_t width, uint32_t height);

        float GetOrthoSize() const { return m_OrthoSize; }
        void SetOrthoSize(float size) { m_OrthoSize = size; RecalculateProjection(); }
        float GetOrthoNearClip() const { return m_OrthoNear; }
        void SetOrthoNearClip(float nearClip){ m_OrthoNear = nearClip; RecalculateProjection(); }
        float GetOrthoFarClip() const { return m_OrthoFar; }
        void SetOrthoFarClip(float farClip) { m_OrthoFar = farClip; RecalculateProjection(); }

        float GetPerspFOV() const { return m_PerspFOV; }
        void SetPerspFOV(float FOV) { m_PerspFOV = FOV; RecalculateProjection(); }
        float GetPerspNearClip() const { return m_PerspNear; }
        void SetPerspNearClip(float nearClip) { m_PerspNear = nearClip; RecalculateProjection(); }
        float GetPerspFarClip() const { return m_PerspFar; }
        void SetPerspFarClip(float farClip) { m_PerspFar = farClip; RecalculateProjection(); }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

    private:
        void RecalculateProjection();
    private:
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;
        float m_AspectRatio = 0.0f;

        float m_PerspFOV = glm::radians(45.0f);
        float m_PerspNear = 0.01f, m_PerspFar = 1000.0f;

        float m_OrthoSize = 10.0f;
        float m_OrthoNear = -10.0f, m_OrthoFar = 10.0f;
    };

}
