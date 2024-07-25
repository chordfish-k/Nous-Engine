#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

SandBox2D::SandBox2D()
    : Layer("SandBox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void SandBox2D::OnAttached()
{
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

    m_FlatColorShader = Nous::Shader::Create("assets/shaders/FlatColor.glsl");
}

void SandBox2D::OnDetached()
{
}

void SandBox2D::OnUpdate(Nous::Timestep dt)
{
    // Update
    m_CameraController.OnUpdate(dt);

    // Render
    Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Nous::RenderCommand::Clear();

    Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
    Nous::Renderer2D::DrawQuad({0.0f, 0.0f}, {0.5f, 0.5f}, {0.8f, 0.2f, 0.3f, 1.0f});
    Nous::Renderer2D::DrawQuad({0.5f, 0.0f}, {0.5f, 0.5f}, {0.2f, 0.8f, 0.3f, 1.0f});
    Nous::Renderer2D::DrawQuad({0.5f, 0.5f}, {0.5f, 0.5f}, {0.2f, 0.3f, 0.8f, 1.0f});
    Nous::Renderer2D::EndScene();
}

void SandBox2D::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

void SandBox2D::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}