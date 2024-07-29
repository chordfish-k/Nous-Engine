#include "pch.h"
#include "Renderer2D.h"

#include "Nous/Renderer/VertexArray.h"
#include "Nous/Renderer/Shader.h"
#include "Nous/Renderer/RenderCommand.h"

#include <glm/ext/matrix_transform.hpp>

namespace Nous {

    struct Renderer2DStorage
    {
        Ref <VertexArray> QuadVertexArray;
        Ref <Shader> TextureShader;
        Ref <Texture2D> WhiteTexture;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init()
    {
        NS_PROFILE_FUNCTION();

        s_Data = new Renderer2DStorage();
        s_Data->QuadVertexArray = Nous::VertexArray::Create();

        // 正方形
        float squareVertices[7 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        };

        Ref <Nous::VertexBuffer> squareVB;
        squareVB = Nous::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
        squareVB->SetLayout(
            {
                {Nous::ShaderDataType::Float3, "a_Position"},
                {Nous::ShaderDataType::Float2, "a_TexCoord"}
            });
        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Ref <Nous::IndexBuffer> squareIB;
        squareIB = Nous::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        // 生成白色纹理
        s_Data->WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        s_Data->TextureShader = Nous::Shader::Create("assets/shaders/Texture.glsl");
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetInt("u_Texture", 0);
    }

    void Renderer2D::Shutdown()
    {
        NS_PROFILE_FUNCTION();

        delete s_Data;
    }

    void Renderer2D::BeginScene(const Camera& camera)
    {
        NS_PROFILE_FUNCTION();

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    }

    void Renderer2D::EndScene()
    {
        NS_PROFILE_FUNCTION();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetFloat4("u_Color", color);
        // 绑定白色纹理
        s_Data->WhiteTexture->Bind();

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data->TextureShader->SetMat4("u_Transform", transform);
        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref <Texture2D>& texture, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref <Texture2D>& texture, const glm::vec4& color)
    {
        NS_PROFILE_FUNCTION();

        s_Data->TextureShader->Bind();

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data->TextureShader->SetMat4("u_Transform", transform);
        s_Data->TextureShader->SetFloat4("u_Color", color);
        texture->Bind();
        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }
}