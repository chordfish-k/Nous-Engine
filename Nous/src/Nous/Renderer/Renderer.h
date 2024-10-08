﻿#pragma once

#include "Nous/Renderer/RenderCommand.h"
#include "Nous/Renderer/Shader.h"
#include "Nous/Renderer/OrthoCamera.h"

namespace Nous {

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene(OrthoCamera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

    private:
        struct SceneData
        {
            // 相机在帧内不移动，记录其矩阵的缓存
            glm::mat4 ViewProjectionMatrix;
        };

        static Scope<SceneData> m_SceneData;
    };

}
