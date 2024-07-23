#pragma once

#include "Nous/Renderer/Camera.h"
#include "Nous/Core/Timestep.h"

#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/MouseButtonEvent.h"

namespace Nous {

    class CameraController
    {
    public:
        CameraController(float aspectRatio, bool enableRotation = true);

        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);

        Camera& GetCamera() { return m_Camera; };
        const Camera& GetCamera() const { return m_Camera; };

        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLevel(float level) { m_ZoomLevel = level; }
    private:
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
    private:
        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;
        Camera m_Camera;

        bool m_EnableRotation;
        glm::vec3 m_CameraPosition = {0.0f, 0.0f, 0.0f};
        float m_CameraRotation = 0.0f; //角度制，逆时针
        float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
    };

}
