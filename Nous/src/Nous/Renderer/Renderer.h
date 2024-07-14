#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "Camera.h"

namespace Nous {

    class Renderer
    {
    public:
        static void BeginScene(Camera& camera);

        static void EndScene();

        static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

    private:
        struct SceneData
        {
            // 相机在帧内不移动，记录其矩阵的缓存
            glm::mat4 ViewProjectionMatrix;
        };
        
        static SceneData* m_SceneData;
    };

}
