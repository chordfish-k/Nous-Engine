#pragma once

#include "Nous/Renderer/Camera.h"

namespace Nous {

    // 场景摄像机
    class SceneCamera : public Camera
    {
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetOrtho(float size, float nearClip, float farClip);
        void SetViewportSize(uint32_t width, uint32_t height);

        float GetOrthoSize() const { return m_OrthoSize; }
        void SetOrthoSize(float size);
    private:
        void RecalculateProjection();
    private:
        float m_AspectRatio = 0.0f;

        float m_OrthoSize = 10.0f;
        float m_OrthoNear = -1.0f, m_OrthoFar = 1.0f;
    };

}
