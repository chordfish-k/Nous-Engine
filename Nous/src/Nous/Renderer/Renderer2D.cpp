#include "pch.h"
#include "Renderer2D.h"

#include "Nous/Renderer/VertexArray.h"
#include "Nous/Renderer/Shader.h"
#include "Nous/Renderer/UniformBuffer.h"
#include "Nous/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Nous {

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
        int EntityID;
    };

    struct CircleVertex
    {
        glm::vec3 WorldPosition;
        glm::vec3 LocalPosition;
        glm::vec4 Color;
        float Thickness;
        float Fade;
        int EntityID;
    };

    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        int EntityID;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 20000; // 批次渲染的最大四边形数量
        static const uint32_t MaxVertices = MaxQuads * 4; // 四边形的顶点个数
        static const uint32_t MaxIndices = MaxQuads * 6; // 四边形的索引个数 (0 1 2 2 3 0)
        static const uint32_t MaxTextureSlots = 32; // TODO 应该获取渲染器的上限

        Ref <VertexArray> QuadVertexArray;
        Ref <VertexBuffer> QuadVertexBuffer;
        Ref <Shader> QuadShader;
        Ref <Texture2D> WhiteTexture;

        Ref <VertexArray> CircleVertexArray;
        Ref <VertexBuffer> CircleVertexBuffer;
        Ref <Shader> CircleShader;

        Ref <VertexArray> LineVertexArray;
        Ref <VertexBuffer> LineVertexBuffer;
        Ref <Shader> LineShader;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr; // 缓冲区基地址
        QuadVertex* QuadVertexBufferPtr = nullptr; // 缓冲区读写指针

        uint32_t CircleIndexCount = 0;
        CircleVertex* CircleVertexBufferBase = nullptr;
        CircleVertex* CircleVertexBufferPtr = nullptr;

        uint32_t LineVertexCount = 0;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;

        float LineWidth = 2.0f;

        std::array<Ref < Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = 白色纹理

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Statistics Stats;

        struct CameraData
        {
            glm::mat4 ViewProjection;
        };
        CameraData CameraBuffer;
        Ref <UniformBuffer> CameraUniformBuffer;
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        NS_PROFILE_FUNCTION();

        // Quads
        s_Data.QuadVertexArray = VertexArray::Create();

        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout(
            {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float4, "a_Color"},
                {ShaderDataType::Float2, "a_TexCoord"},
                {ShaderDataType::Float,  "a_TexIndex"},
                {ShaderDataType::Float,  "a_TilingFactor"},
                {ShaderDataType::Int,    "a_EntityID"},
            });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4; // 偏移为顶点数
        }

        Ref <IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        // Circles
        s_Data.CircleVertexArray = VertexArray::Create();

        s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
        s_Data.CircleVertexBuffer->SetLayout(
            {
                {ShaderDataType::Float3, "a_WorldPosition"},
                {ShaderDataType::Float3, "a_LocalPosition"},
                {ShaderDataType::Float4, "a_Color"},
                {ShaderDataType::Float,  "a_Thickness"},
                {ShaderDataType::Float,  "a_Fade"},
                {ShaderDataType::Int,    "a_EntityID"}
            });
        s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->SetIndexBuffer(quadIB); // Use quad IB
        s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

        // Lines
        s_Data.LineVertexArray = VertexArray::Create();

        s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
        s_Data.LineVertexBuffer->SetLayout(
            {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float4, "a_Color"},
                {ShaderDataType::Int,    "a_EntityID"}
            });
        s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];


        // 生成白色纹理
        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
        s_Data.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
        s_Data.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");

        // 设置0号槽为白色纹理
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
    }

    void Renderer2D::Shutdown()
    {
        NS_PROFILE_FUNCTION();

        delete[] s_Data.QuadVertexBufferBase;
    }

    void Renderer2D::BeginScene(const OrthoCamera& camera)
    {
        NS_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        NS_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = camera.GetProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
        NS_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::EndScene()
    {
        NS_PROFILE_FUNCTION();

        Flush();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush()
    {
        if (s_Data.QuadIndexCount)
        {
            uint32_t dataSize = (uint32_t) ((uint8_t*) s_Data.QuadVertexBufferPtr -
                                            (uint8_t*) s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

            // 纹理绑定
            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->Bind(i);

            s_Data.QuadShader->Bind();
            RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.CircleIndexCount)
        {
            uint32_t dataSize = (uint32_t) ((uint8_t*) s_Data.CircleVertexBufferPtr -
                                            (uint8_t*) s_Data.CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

            s_Data.CircleShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.LineVertexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
            s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

            s_Data.LineShader->Bind();
            RenderCommand::SetLineWidth(s_Data.LineWidth);
            RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
            s_Data.Stats.DrawCalls++;
        }
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, texture, tilingFactor, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});


        DrawQuad(transform, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& color)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, texture, tilingFactor, color);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
    {
        NS_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f},
                                               {1.0f, 0.0f},
                                               {1.0f, 1.0f},
                                               {0.0f, 1.0f}};

        const float textureIndex = 0.0f;
        const float tilingFactor = 1.0f;

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& color, int entityID)
    {
        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f},
                                               {1.0f, 0.0f},
                                               {1.0f, 1.0f},
                                               {0.0f, 1.0f}};

        float textureIndex = 0.0f;
        // 找出当前纹理的id
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i] == *texture.get())
            {
                textureIndex = (float) i;
                break;
            }
        }

        // 如果是新纹理，则添加到纹理槽
        if (textureIndex == 0.0f)
        {
            // TODO 待优化，可能应该同一张纹理一批
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float) s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade, int entityID)
    {
        NS_PROFILE_FUNCTION();

        // TODO: 实现圆形的整批绘制
        // if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        // 	NextBatch();

        for (size_t i = 0; i < 4; i++)
        {
            s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = color;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr->EntityID = entityID;
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID)
    {
        s_Data.LineVertexBufferPtr->Position = p0;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->Position = p1;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
    }

    void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
    {
        glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
        glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);

    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
    {
        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

        DrawLine(lineVertices[0], lineVertices[1], color);
        DrawLine(lineVertices[1], lineVertices[2], color);
        DrawLine(lineVertices[2], lineVertices[3], color);
        DrawLine(lineVertices[3], lineVertices[0], color);
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, CSpriteRenderer& src, int entityID)
    {
        if (src.Texture)
            DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
        else
            DrawQuad(transform, src.Color, entityID);
    }

    float Renderer2D::GetLineWidth()
    {
        return s_Data.LineWidth;
    }

    void Renderer2D::SetLineWidth(float width)
    {
        s_Data.LineWidth = width;
    }

    // 重置统计数据
    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    // 获取统计数据
    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }
}