#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/type_ptr.hpp>

#include <chrono>

SandBox2D::SandBox2D()
    : Layer("SandBox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void SandBox2D::OnAttached()
{
    NS_PROFILE_FUNCTION();

    m_MarioTexture = Nous::Texture2D::Create("assets/textures/Mario.png");
}

void SandBox2D::OnDetached()
{
    NS_PROFILE_FUNCTION();
}

static float fps = 0.0f;

void SandBox2D::OnUpdate(Nous::Timestep dt)
{
    NS_PROFILE_FUNCTION();

    fps = 1.0f / dt;

    // Update
    m_CameraController.OnUpdate(dt);

    // Render
    Nous::Renderer2D::ResetStats();
    {
        NS_PROFILE_SCOPE("Render Prepare");
        Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Nous::RenderCommand::Clear();
    }
    {
        NS_PROFILE_SCOPE("Render Draw");

        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        Nous::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
        Nous::Renderer2D::DrawQuad({ 4.8f, -0.5f, -0.1f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
        Nous::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_MarioTexture, 10.0f);
        Nous::Renderer2D::DrawRotatedQuad({ 0.0f, 0.0f, 0.1f }, { 1.0f, 1.0f }, 30.0, m_MarioTexture, 20.0f);
        Nous::Renderer2D::EndScene();

        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        const float width = 0.5f;
        for (float y = -5.0f; y < 5.0f; y += width)
        {
            for (float x = -5.0f; x < 5.0f; x += width)
            {
                glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f };
                Nous::Renderer2D::DrawQuad({ x, y }, { width, width }, color);
            }
        }
        Nous::Renderer2D::EndScene();
    }
}

void SandBox2D::OnImGuiRender()
{
    NS_PROFILE_FUNCTION();

    ImGui::Begin("Settings");

    auto stats = Nous::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::Text("FPS: %.2f", fps);

    ImGui::End();
}

void SandBox2D::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}