﻿#include "pch.h"
#include "CameraController.h"

#include "Nous/KeyCodes.h"
#include "Nous/Input.h"

namespace Nous {

    CameraController::CameraController(float aspectRatio, bool enableRotation)
        : m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
          m_AspectRatio(aspectRatio), m_EnableRotation(enableRotation)
    {

    }

    void CameraController::OnUpdate(Timestep ts)
    {
        if (Input::IsKeyPressed(NS_KEY_A))
            m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
        else if (Input::IsKeyPressed(NS_KEY_D))
            m_CameraPosition.x += m_CameraTranslationSpeed * ts;

        if (Input::IsKeyPressed(NS_KEY_W))
            m_CameraPosition.y += m_CameraTranslationSpeed * ts;
        else if (Input::IsKeyPressed(NS_KEY_S))
            m_CameraPosition.y -= m_CameraTranslationSpeed * ts;

        if (m_EnableRotation)
        {
            if (Input::IsKeyPressed(NS_KEY_Q))
                m_CameraRotation += m_CameraRotationSpeed * ts;
            if (Input::IsKeyPressed(NS_KEY_E))
                m_CameraRotation -= m_CameraRotationSpeed * ts;

            m_Camera.SetRotation(m_CameraRotation);
        }

        m_Camera.SetPosition(m_CameraPosition);

        m_CameraTranslationSpeed = m_ZoomLevel;
    }

    void CameraController::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(NS_BIND_EVENT_FN(CameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(NS_BIND_EVENT_FN(CameraController::OnWindowResized));
    }

    bool CameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_ZoomLevel -= e.GetYOffset() * 0.5f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    bool CameraController::OnWindowResized(WindowResizeEvent& e)
    {
        m_AspectRatio = (float) e.GetWidth() / (float) e.GetHeight();
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        return false;
    }
}
