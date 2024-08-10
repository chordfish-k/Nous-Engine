#pragma once

#include "Nous/Renderer/Camera.h"
#include "Nous/Renderer/OrthoCamera.h"
#include "Nous/Renderer/Texture.h"
#include "Nous/Renderer/EditorCamera.h"
#include "Nous/Scene/Component.h"

namespace Nous {

    class Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const OrthoCamera& camera);
        static void EndScene();
        static void Flush();

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f));

        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
        static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1);

        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tintColor);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tintColor);
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f));
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& color = glm::vec4(1.0f));

        static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

        static void DrawSprite(const glm::mat4& transform, CSpriteRenderer& src, int entityID = -1);

        // 统计数据
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
        };
        static Statistics GetStats();
        static void ResetStats();
    private:
        static void StartBatch();
        static void NextBatch();
    };

}

