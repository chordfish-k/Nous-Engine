#pragma once

#include "Nous/Renderer/OrthoCamera.h"
#include "Nous/Core/Timestep.h"

#include "Nous/Event/ApplicationEvent.h"
#include "Nous/Event/MouseEvent.h"

namespace Nous {

    class OrthoCameraController
    {
    public:
        OrthoCameraController(float aspectRatio, bool enableRotation = true);

        void OnUpdate(Timestep ts);
        void OnEvent(Event& e);

        void OnResize(float width, float height);

        OrthoCamera& GetCamera() { return m_Camera; };
        const OrthoCamera& GetCamera() const { return m_Camera; };

        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLevel(float level) { m_ZoomLevel = level; }
    private:
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
    private:
        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;
        OrthoCamera m_Camera;

        bool m_EnableRotation;
        glm::vec3 m_CameraPosition = {0.0f, 0.0f, 0.0f};
        float m_CameraRotation = 0.0f; //角度制，逆时针
        float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
    };

}
