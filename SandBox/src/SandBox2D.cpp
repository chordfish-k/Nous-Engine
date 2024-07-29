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

void SandBox2D::OnUpdate(Nous::Timestep dt)
{
    NS_PROFILE_FUNCTION();
    // Update

    m_CameraController.OnUpdate(dt);

    // Render
    {
        NS_PROFILE_SCOPE("Render Prepare");
        Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Nous::RenderCommand::Clear();
    }
    {
        NS_PROFILE_SCOPE("Render Draw");
        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        // Hazel::Renderer2D::DrawRotatedQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(-45.0f), { 0.8f, 0.2f, 0.3f, 1.0f });
        Nous::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
        Nous::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
        // Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10.0f);
        Nous::Renderer2D::EndScene();
    }
}

void SandBox2D::OnImGuiRender()
{
    NS_PROFILE_FUNCTION();

    ImGui::Begin("Settings");
    ImGui::End();
}

void SandBox2D::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}