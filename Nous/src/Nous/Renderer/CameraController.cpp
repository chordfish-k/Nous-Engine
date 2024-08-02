#include "pch.h"
#include "CameraController.h"

#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/Input.h"

namespace Nous {

    CameraController::CameraController(float aspectRatio, bool enableRotation)
        : m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
          m_AspectRatio(aspectRatio), m_EnableRotation(enableRotation)
    {

    }

    void CameraController::OnUpdate(Timestep ts)
    {
        NS_PROFILE_FUNCTION();

        if (Input::IsKeyPressed(NS_KEY_A))
        {
            m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }
        else if (Input::IsKeyPressed(NS_KEY_D))
        {
            m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }

        if (Input::IsKeyPressed(NS_KEY_W))
        {
            m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }
        else if (Input::IsKeyPressed(NS_KEY_S))
        {
            m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }

        if (m_EnableRotation)
        {
            if (Input::IsKeyPressed(NS_KEY_Q))
                m_CameraRotation += m_CameraRotationSpeed * ts;

            if (Input::IsKeyPressed(NS_KEY_E))
                m_CameraRotation -= m_CameraRotationSpeed * ts;

            if (m_CameraRotation > 180.0f)
                m_CameraRotation -= 360.0f;
            else if (m_CameraRotation <= -180.0f)
                m_CameraRotation += 360.0f;

            m_Camera.SetRotation(m_CameraRotation);
        }

        m_Camera.SetPosition(m_CameraPosition);

        m_CameraTranslationSpeed = m_ZoomLevel;
    }

    void CameraController::OnEvent(Event& e)
    {
        NS_PROFILE_FUNCTION();

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(NS_BIND_EVENT_FN(CameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(NS_BIND_EVENT_FN(CameraController::OnWindowResized));
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        NS_PROFILE_FUNCTION();

        m_ZoomLevel -= e.GetYOffset() * 0.5f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    bool CameraController::OnWindowResized(WindowResizeEvent& e)
    {
        NS_PROFILE_FUNCTION();

        OnResize(e.GetWidth(), e.GetHeight());
        return false;
    }

    void CameraController::OnResize(float width, float height)
    {
        m_AspectRatio = (float) width / (float) height;
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
    }
}
