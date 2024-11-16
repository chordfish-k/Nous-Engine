#pragma once

#include "Nous/Renderer/Camera.h"
#include "Nous/Core/Timestep.h"
#include "Nous/Event/Event.h"
#include "Nous/Event/MouseEvent.h"
#include "Nous/Event/KeyEvent.h"

#include <glm/glm.hpp>

namespace Nous {

    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

        void OnUpdate(Timestep dt);
        void OnEvent(Event& e);

        float GetDistance() const { return m_Distance; }
        void SetDistance(float distance) { m_Distance = distance; }

        void SetViewportSize(float width, float height);
        float GetAspectRatio() const { return m_ViewportWidth / m_ViewportHeight; };

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetViewProjectionMatrix() const { return m_Projection * m_ViewMatrix; }

        glm::vec3 GetUpDirection() const; // 方向向量 - 上
        glm::vec3 GetRightDirection() const; // 方向向量 - 右
        glm::vec3 GetForwardDirection() const; // 方向向量 - 前

        const glm::vec3& GetPosition() const { return m_Position; }
        // 获取四元数表示的方向
        glm::quat GetOrientation() const;

        float GetPitch() const { return m_Pitch; }
        float GetYaw() const { return m_Yaw; }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        void Reset();
    private:
        void RecalculateProjection();
        void RecalculateView();

        bool OnMouseScroll(MouseScrolledEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);

        void MousePan(const glm::vec2& delta); // 视窗操作 - 平移
        void MouseRotate(const glm::vec2& delta); // 视窗操作 - 旋转
        void MouseZoom(float delta); // 视窗操作 - 缩放

        glm::vec3 CalculatePosition() const;

        std::pair<float, float> PanSpeed() const; // 根据视窗大小获取平移速度
        float RotationSpeed() const;
        float ZoomSpeed() const;

    private:
        float m_FOV = 45.0f, m_AspectRatio = 1.778f /* =16/9 */, m_NearClip = 0.1f, m_FarClip = 1000.0f;
        float m_OrthoNearClip = -1000.0f, m_OrthoFarClip = 1000.0f;

        glm::mat4 m_ViewMatrix {1.0f};
        glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_FocalPoint = {0.0f, 0.0f, 0.0f}; // 焦点位置

        glm::vec2 m_InitialMousePosition = {0.0f, 0.0f};

        float m_Distance = 15.0f;
        float m_Pitch = 0.0f, m_Yaw = 0.0f; // 俯仰角， 水平角
        float m_ViewportWidth = 1280, m_ViewportHeight = 720;

        
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;
    };

}

