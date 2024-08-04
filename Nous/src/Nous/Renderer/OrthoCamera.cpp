#include "pch.h"
#include "OrthoCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nous {

    OrthoCamera::OrthoCamera(float left, float right, float bottom, float top)
        : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
    {
        NS_PROFILE_FUNCTION();

        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthoCamera::RecalculateViewMatrix()
    {
        NS_PROFILE_FUNCTION();

        // 由于是2d相机，不需要指定front/right等三维属性
        // 先将相机变换到世界参考系，再求逆，得到视图矩阵
        // 1.平移相机到世界参考系 2.旋转相机。旋转轴为z
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
                              * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
        // 求逆得到正确的视图矩阵，世界参考系->相机参考系
        m_ViewMatrix = glm::inverse(transform);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthoCamera::SetProjection(float left, float right, float bottom, float top)
    {
        NS_PROFILE_FUNCTION();

        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}