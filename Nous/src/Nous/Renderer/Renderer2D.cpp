#include "pch.h"
#include "Renderer2D.h"

#include "Nous/Renderer/VertexArray.h"
#include "Nous/Renderer/Shader.h"
#include "Nous/Renderer/RenderCommand.h"

#include <glm/ext/matrix_transform.hpp>

namespace Nous {

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct Renderer2DData
    {
        const uint32_t MaxQuads = 10000; // 批次渲染的最大四边形数量
        const uint32_t MaxVertices = MaxQuads * 4; // 四边形的顶点个数
        const uint32_t MaxIndices = MaxQuads * 6; // 四边形的索引个数 (0 1 2 2 3 0)
        static const uint32_t MaxTextureSlots = 32; // TODO 应该获取渲染器的上限

        Ref <VertexArray> QuadVertexArray;
        Ref <VertexBuffer> QuadVertexBuffer;
        Ref <Shader> TextureShader;
        Ref <Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr; // 缓冲区基地址
        QuadVertex* QuadVertexBufferPtr = nullptr; // 缓冲区读写指针

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = 白色纹理

        glm::vec4 QuadVertexPositions[4]{};
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init()
    {
        NS_PROFILE_FUNCTION();

        s_Data.QuadVertexArray = Nous::VertexArray::Create();

        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout(
            {
                {Nous::ShaderDataType::Float3, "a_Position"},
                {Nous::ShaderDataType::Float4, "a_Color"},
                {Nous::ShaderDataType::Float2, "a_TexCoord"},
                {Nous::ShaderDataType::Float, "a_TexIndex"},
                {Nous::ShaderDataType::Float, "a_TilingFactor"}
            });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        uint32_t * quadIndices = new uint32_t[s_Data.MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i+= 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;
            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4; // 偏移为顶点数
        }

        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);

        delete[] quadIndices;

        // 生成白色纹理
        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data.TextureShader = Nous::Shader::Create("assets/shaders/Texture.glsl");
        s_Data.TextureShader->Bind();

        s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        // 设置0号槽为白色纹理
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
    }

    void Renderer2D::Shutdown()
    {
        NS_PROFILE_FUNCTION();
    }

    void Renderer2D::BeginScene(const Camera& camera)
    {
        NS_PROFILE_FUNCTION();

        // 设置默认值
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::EndScene()
    {
        NS_PROFILE_FUNCTION();

        uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
        s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

        Flush();
    }

    void Renderer2D::Flush()
    {
        // 绑定纹理
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
        {
            s_Data.TextureSlots[i]->Bind(i);
        }

        RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        const float textureIndex = 0.0f;
        const float tilingFactor = 1.0f;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        // 在缓冲区添加四边形四个顶点的信息，不是立即绘制
        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
        s_Data.QuadVertexBufferPtr->Color = color;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        NS_PROFILE_FUNCTION();

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
            textureIndex = (float) s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tintColor)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tintColor)
    {
        NS_PROFILE_FUNCTION();

        const float textureIndex = 0.0f;
        const float tilingFactor = 1.0f;

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                              * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
                              * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref <Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        NS_PROFILE_FUNCTION();

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
            textureIndex = (float) s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[0];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[1];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[2];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[3];
        s_Data.QuadVertexBufferPtr->Color = tintColor;
        s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
        s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;
    }
}