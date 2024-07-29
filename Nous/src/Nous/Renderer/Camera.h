#pragma once

#include <glm/glm.hpp>

namespace Nous {

    /* 相机(正交)
    *
    * 屏幕坐标 = P * V * M * Vec
    *
    */
    class Camera
    {
    public:
        Camera(float left, float right, float bottom, float top);

        void SetProjection(float left, float right, float bottom, float top);

        inline const glm::vec3& GetPosition() const { return m_Position; }

        void SetPosition(const glm::vec3& position)
        {
            m_Position = position;
            RecalculateViewMatrix();
        }

        inline float GetRotation() const { return m_Rotation; }

        void SetRotation(float rotation)
        {
            m_Rotation = rotation;
            RecalculateViewMatrix();
        }

        inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

        inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

        inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    protected:
        void RecalculateViewMatrix();

    protected:
        // 定义了一个类似立方体的平截头箱，它定义了一个裁剪空间，在这空间之外的顶点都会被裁剪掉
        glm::mat4 m_ProjectionMatrix;
        // 将场景中的物体从世界空间转换到摄像机空间，即从世界坐标系转换到摄像机坐标系。
        glm::mat4 m_ViewMatrix;
        // 视图投影矩阵 VP = P * V
        glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
        float m_Rotation = 0.0f;
    };
}

