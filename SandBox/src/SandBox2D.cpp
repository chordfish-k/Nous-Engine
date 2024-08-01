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
    m_CheckerboardTexture = Nous::Texture2D::Create("assets/textures/Checkerboard.png");

    Nous::FramebufferSpecification fbSpec = {};
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    m_Framebuffer = Nous::Framebuffer::Create(fbSpec);
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
        m_Framebuffer->Bind();
        Nous::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Nous::RenderCommand::Clear();
    }
    {
        NS_PROFILE_SCOPE("Render Draw");

        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        Nous::Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f, 0.8f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Nous::Renderer2D::DrawQuad({4.8f, -0.5f, -0.1f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
        Nous::Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {10.0f, 10.0f}, m_MarioTexture, 10.0f);
        Nous::Renderer2D::DrawRotatedQuad({0.0f, 0.0f, 0.1f}, {1.0f, 1.0f}, 30.0, m_MarioTexture, 20.0f);
        Nous::Renderer2D::EndScene();

        Nous::Renderer2D::BeginScene(m_CameraController.GetCamera());
        const float width = 0.5f;
        for (float y = -5.0f; y < 5.0f; y += width)
        {
            for (float x = -5.0f; x < 5.0f; x += width)
            {
                glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.5f};
                Nous::Renderer2D::DrawQuad({x, y}, {width, width}, color);
            }
        }
        Nous::Renderer2D::EndScene();
        m_Framebuffer->Unbind();
    }
}

void SandBox2D::OnImGuiRender()
{
    NS_PROFILE_FUNCTION();

    static bool dockingEnable = true;
    if (dockingEnable)
    {
        static bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkPos());
            ImGui::SetNextWindowSize(viewport->GetWorkSize());
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|
                            ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoNavFocus;
        } else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags&ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags&ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                if (ImGui::MenuItem("Exit", "", (dockspace_flags&ImGuiDockNodeFlags_NoSplit) != 0))
                    Nous::Application::Get().Close();

                ImGui::Separator();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Begin("Settings");

        auto stats = Nous::Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::Text("FPS: %.2f", fps);

        ImGui::End();

        ImGui::Begin("Viewport");
        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*) textureID, ImVec2{1280, 720});
        ImGui::End();

        ImGui::End();
    }
}

void SandBox2D::OnEvent(Nous::Event& e)
{
    m_CameraController.OnEvent(e);
}