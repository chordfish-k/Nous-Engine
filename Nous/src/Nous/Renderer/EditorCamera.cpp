#include "pch.h"
#include "EditorCamera.h"

#include "Nous/Core/Input.h"
#include "Nous/Core/KeyCodes.h"
#include "Nous/Core/MouseCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Nous {
    // 参考1: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/EditorCamera.cpp
    // 参考2： https://www.jianshu.com/p/c5d8fb2ae5bb

    EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
    {
        RecalculateView();
    }

    void EditorCamera::RecalculateProjection()
    {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        }
        else
        {
            // 正交模式下，根据摄像机的位置和焦点之间的距离调整正交尺寸
            constexpr float sizeMul = 0.4f;
            float left = -m_AspectRatio * m_Distance * sizeMul;
            float right = m_AspectRatio * m_Distance * sizeMul;
            float bottom = -m_Distance * sizeMul;
            float top = m_Distance * sizeMul;

            // 更新正交投影的参数
            m_Projection = glm::ortho(left, right, bottom, top, m_OrthoNearClip, m_OrthoFarClip);
        }
    }


    void EditorCamera::RecalculateView()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            // 透视模式下，根据距离计算摄像机位置
            m_Position = CalculatePosition();
            glm::quat orientation = GetOrientation();
            m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
            m_ViewMatrix = glm::inverse(m_ViewMatrix);
        }
        else
        {
            m_Position = CalculatePosition();
            glm::quat orientation = GetOrientation();
            m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
            m_ViewMatrix = glm::inverse(m_ViewMatrix);
        }
    }


    std::pair<float, float> EditorCamera::PanSpeed() const
    {
        float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
        float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
        float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { xFactor, yFactor };
    }

    float EditorCamera::RotationSpeed() const
    {
        return 0.8f;
    }

    float EditorCamera::ZoomSpeed() const
    {
        float distance = m_Distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f); // max speed = 100
        return speed;
    }

    void EditorCamera::OnUpdate(Timestep dt)
    {
        if (Input::IsKeyPressed(Key::LeftAlt))
        {
            const glm::vec2& mouse { Input::GetMouseX(), Input::GetMouseY() };
            glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse;

            if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
                MousePan(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
                MouseRotate(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
                MouseZoom(delta.y);

            RecalculateView();
        }
    }

    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(NS_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
        dispatcher.Dispatch<KeyPressedEvent>(NS_BIND_EVENT_FN(EditorCamera::OnKeyPressed));
    }

    void EditorCamera::SetViewportSize(float width, float height)
    {
        if (m_ViewportWidth == width && m_ViewportHeight == height)
            return;
        m_ViewportWidth = width; 
        m_ViewportHeight = height; 
        RecalculateProjection(); 
    }

    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
    {
        float delta = e.GetYOffset() * 0.1f;
        MouseZoom(delta);
        RecalculateView();
        return false;
    }

    bool EditorCamera::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.IsRepeat())
            return false;

        switch (e.GetKeyCode())
        {
            case NS_KEY_8:
            case NS_KEY_KP_8:
                m_ProjectionType = ProjectionType::Orthographic;
                break;
            case NS_KEY_9:
            case NS_KEY_KP_9:
                m_ProjectionType = ProjectionType::Perspective;
                break;
            case NS_KEY_0:
            case NS_KEY_KP_0:
                Reset();
                break;
        }
        RecalculateProjection();
        RecalculateView();
        return false;
    }

    void EditorCamera::MousePan(const glm::vec2& delta)
    {
        auto [xSpeed, ySpeed] = PanSpeed();
        // 根据自身朝向来平移
        m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
    }

    void EditorCamera::MouseRotate(const glm::vec2& delta)
    {
        float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yawSign * delta.x * RotationSpeed();
        m_Pitch += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta)
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            // 透视模式下的缩放，影响摄像机距离
            m_Distance -= delta * ZoomSpeed();
            if (m_Distance < 1.0f)
            {
                m_FocalPoint += GetForwardDirection();
                m_Distance = 1.0f;
            }
        }
        else
        {
            // 正交模式下的缩放，影响 m_Distance 来控制视图范围
            m_Distance -= delta * ZoomSpeed();
            if (m_Distance < 0.1f)
                m_Distance = 0.1f;
        }
        RecalculateProjection();
    }

    glm::vec3 EditorCamera::GetUpDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetRightDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetForwardDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 EditorCamera::CalculatePosition() const
    {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat EditorCamera::GetOrientation() const
    {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    void EditorCamera::Reset()
    {
        m_ViewMatrix = glm::mat4{1.0f};
        m_Position = {0.0f, 0.0f, 0.0f};
        m_FocalPoint = {0.0f, 0.0f, 0.0f}; // 焦点位置

        m_InitialMousePosition = {0.0f, 0.0f};

        m_Distance = 15.0f;
        m_Pitch = 0.0f;
        m_Yaw = 0.0f; // 俯仰角， 水平角

        RecalculateView();
    }
}