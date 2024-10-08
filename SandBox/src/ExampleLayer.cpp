﻿#include "pch.h"
#include "ExampleLayer.h"

#include "imgui/imgui.h"
#include "ext/matrix_transform.hpp"

#include "Platform/OpenGL/OpenGLShader.h"
#include "gtc/type_ptr.hpp"

ExampleLayer::ExampleLayer()
    : Layer("Example"), m_CameraController(1280.0f / 720.0f)
{
    // 三角形
    m_VertexArray = Nous::VertexArray::Create();
    float vertices[3 * 7] = {
        -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
        0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
        0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
    };
    Nous::Ref<Nous::VertexBuffer> vertexBuffer;
    vertexBuffer = Nous::VertexBuffer::Create(vertices, sizeof(vertices));
    Nous::BufferLayout layout = {
        {Nous::ShaderDataType::Float3, "a_Position"},
        {Nous::ShaderDataType::Float4, "a_Color"}
    };
    vertexBuffer->SetLayout(layout);
    m_VertexArray->AddVertexBuffer(vertexBuffer);

    uint32_t indices[3] = {0, 1, 2};
    Nous::Ref<Nous::IndexBuffer> indexBuffer;
    indexBuffer = Nous::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
    m_VertexArray->SetIndexBuffer(indexBuffer);

    // 正方形
    m_SquareVA = Nous::VertexArray::Create();
    float squareVertices[5 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };
    Nous::Ref<Nous::VertexBuffer> squareVB;
    squareVB = Nous::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout(
        {
            {Nous::ShaderDataType::Float3, "a_Position"},
            {Nous::ShaderDataType::Float2, "a_TexCoord"}
        });
    m_SquareVA->AddVertexBuffer(squareVB);
    uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
    Nous::Ref<Nous::IndexBuffer> squareIB;
    squareIB = Nous::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    m_SquareVA->SetIndexBuffer(squareIB);


    // 着色器
    std::string vertexSrc = R"(
            #version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}

        )";
    std::string fragmentSrc = R"(
            #version 330 core
			in vec3 v_Position;
			in vec4 v_Color;

            out vec4 color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}

        )";
    m_Shader = Nous::Shader::Create("VertexColor", vertexSrc, fragmentSrc);

    std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

    std::string flatColorShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

            uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

    m_FlatColorShader = Nous::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);
    auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

    m_Texture = Nous::Texture2D::Create("assets/textures/Checkerboard.png");
    m_MarioTexture = Nous::Texture2D::Create("assets/textures/Mario.png");

    std::dynamic_pointer_cast<Nous::OpenGLShader>(textureShader)->Bind();
    std::dynamic_pointer_cast<Nous::OpenGLShader>(textureShader)->UploadInt("u_Texture", 0);
}

void ExampleLayer::OnUpdate(Nous::Timestep dt)
{
// Update
    m_CameraController.OnUpdate(dt);

// Render
    Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Nous::RenderCommand::Clear();

    Nous::Renderer::BeginScene(m_CameraController.GetCamera());

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

    std::dynamic_pointer_cast<Nous::OpenGLShader>(m_FlatColorShader)->Bind();
    std::dynamic_pointer_cast<Nous::OpenGLShader>(m_FlatColorShader)->UploadFloat3("u_Color", m_SquareColor);

    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
            Nous::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
        }
    }

    auto textureShader = m_ShaderLibrary.Get("Texture");
    m_Texture->Bind();
    Nous::Renderer::Submit(textureShader, m_SquareVA);
    m_MarioTexture->Bind();
    Nous::Renderer::Submit(textureShader, m_SquareVA);

    Nous::Renderer::EndScene();
}

void ExampleLayer::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

void ExampleLayer::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}