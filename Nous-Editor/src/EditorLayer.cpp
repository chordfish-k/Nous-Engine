#include "EditorLayer.h"
#include "imgui/imgui.h"
#include "gtc/type_ptr.hpp"

namespace Nous {

    EditorLayer::EditorLayer()
        : Layer("SandBox2D"), m_CameraController(1280.0f / 720.0f)
    {
    }

    void EditorLayer::OnAttached()
    {
        NS_PROFILE_FUNCTION();

        m_MarioTexture = Texture2D::Create("assets/textures/Mario.png");
        m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");

        FramebufferSpecification fbSpec = {};
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        // Entity
        m_SquareEntity = m_ActiveScene->CreateEntity("Square");
        m_SquareEntity.AddComponent<CSpriteRenderer>(glm::vec4{0.3f, 0.7f, 0.2f, 1.0f});

        m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
        m_CameraEntity.AddComponent<CCamera>(glm::ortho(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f));

        m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
        m_CameraEntity.AddComponent<CCamera>(glm::ortho(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f));

        m_CameraEntity2 = m_ActiveScene->CreateEntity("Clip-Space Entity");
        auto& cc = m_CameraEntity2.AddComponent<CCamera>(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
        cc.Primary = false;
    }

    void EditorLayer::OnDetached()
    {
        NS_PROFILE_FUNCTION();
    }

    static float fps = 0.0f;

    void EditorLayer::OnUpdate(Timestep dt)
    {
        NS_PROFILE_FUNCTION();

        fps = 1.0f / dt;

        // Resize
        auto spec = m_Framebuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != (uint32_t) m_ViewportSize.x ||
             spec.Height != (uint32_t) m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t) m_ViewportSize.x, (uint32_t) m_ViewportSize.y);
            m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Update
        if (m_ViewportFocused)
            m_CameraController.OnUpdate(dt);

        // Render
        Renderer2D::ResetStats();
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();

        // Update Scene
        m_ActiveScene->OnUpdate(dt);

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
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
                        Application::Get().Close();

                    ImGui::Separator();

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            ImGui::Begin("Settings");

            auto stats = Renderer2D::GetStats();
            ImGui::Text("Renderer2D Stats:");
            ImGui::Text("Draw Calls: %d", stats.DrawCalls);
            ImGui::Text("Quads: %d", stats.QuadCount);
            ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
            ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
            ImGui::Text("FPS: %.2f", fps);

            if (m_SquareEntity)
            {
                ImGui::Separator();
                ImGui::Text("%s", m_SquareEntity.GetComponent<CTag>().Tag.c_str());

                auto& squareColor = m_SquareEntity.GetComponent<CSpriteRenderer>().Color;
                ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
                ImGui::Separator();
            }

            ImGui::DragFloat3("Camera Transform",
                              glm::value_ptr(m_CameraEntity.GetComponent<CTransform>().Transform[3]));

            if (ImGui::Checkbox("Camera A", &m_PrimaryCamera))
            {
                m_CameraEntity.GetComponent<CCamera>().Primary = m_PrimaryCamera;
                m_CameraEntity2.GetComponent<CCamera>().Primary = !m_PrimaryCamera;
            }

            ImGui::End();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            ImGui::Begin("Viewport");

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();
            Application::Get().GetImGuiLayer()->SetBlockEvent(!m_ViewportFocused || !m_ViewportHovered);

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

            uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image((void*) textureID, {m_ViewportSize.x, m_ViewportSize.y}, {0, 1}, {1, 0});

            ImGui::End();
            ImGui::PopStyleVar();

            ImGui::End();
        }
    }

    void EditorLayer::OnEvent(Event& e)
    {
        m_CameraController.OnEvent(e);
    }
}
