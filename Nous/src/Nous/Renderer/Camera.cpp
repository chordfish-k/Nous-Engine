﻿#include "pch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nous {

    void Camera::RecalculateViewMatrix()
    {
        // 由于是2d相机，不需要指定front/right等三维属性
        // 先将相机变换到世界参考系，再求逆，得到视图矩阵
        // 1.平移相机到世界参考系 2.旋转相机。旋转轴为z
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
                              * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
        // 求逆得到正确的视图矩阵，世界参考系->相机参考系
        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    Camera::Camera()
        : m_ProjectionMatrix(glm::mat4(1.0f)), m_ViewMatrix(1.0f), m_ViewProjectionMatrix(1.0f)
    {
    }

    Camera::Camera(const glm::vec2& size, const glm::vec3& position)
        : m_ProjectionMatrix(glm::mat4(1.0f)), m_ViewMatrix(1.0f), m_ViewProjectionMatrix(1.0f)
    {
        SetPosition(position);
        SetSize(size * 0.5f);
    }

    Camera::Camera(float left, float right, float bottom, float top)
        : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
    {
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void Camera::SetProjection(float left, float right, float bottom, float top)
    {
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void Camera::SetSize(const glm::vec2& halfSize)
    {
        m_ProjectionMatrix = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

}