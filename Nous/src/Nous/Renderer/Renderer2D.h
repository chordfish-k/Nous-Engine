#pragma once

#include "Nous/Renderer/Camera.h"

namespace Nous {

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const Camera& camera);
        static void EndScene();

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4 color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4 color);
    private:
    };

}

