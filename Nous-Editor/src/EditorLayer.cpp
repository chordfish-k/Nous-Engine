#include "EditorLayer.h"

#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

#include "Panel/DockingSpace.h"

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
        m_SquareEntity = m_ActiveScene->CreateEntity("Green Square");
        m_SquareEntity.AddComponent<CSpriteRenderer>(glm::vec4{0.3f, 0.7f, 0.2f, 1.0f});

        auto redSquare = m_ActiveScene->CreateEntity("Red Square");
        redSquare.AddComponent<CSpriteRenderer>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

        m_CameraEntity = m_ActiveScene->CreateEntity("Camera", {0.0f, 0.0f, 1.0f});
        m_CameraEntity.AddComponent<CCamera>();

        class CameraController : public ScriptableEntity
        {
        public:
            virtual void OnCreate() override
            {
            }

            virtual void OnDestroy() override
            {
            }

            virtual void OnUpdate(Timestep dt) override
            {
                auto& transform = GetComponent<CTransform>().Transform;
                float speed = 5.0f;
                auto& pos = transform[3];
                if (Input::IsKeyPressed(KeyCode::A))
                    pos.x -= speed * dt;
                if (Input::IsKeyPressed(KeyCode::D))
                    pos.x += speed * dt;
                if (Input::IsKeyPressed(KeyCode::W))
                    pos.y += speed * dt;
                if (Input::IsKeyPressed(KeyCode::S))
                    pos.y -= speed * dt;
            }
        };
        m_CameraEntity.AddComponent<CNativeScript>().Bind<CameraController>();

        m_SceneHierarchyPanel.SetContent(m_ActiveScene);
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

            m_ActiveScene->OnViewportResize((uint32_t) m_ViewportSize.x, (uint32_t) m_ViewportSize.y);
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

        DockingSpace::BeginDocking();

        // Properties
        ImGui::Begin("Settings");
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::Text("FPS: %.2f", fps);
        ImGui::End();

        // Hierarchy
        m_SceneHierarchyPanel.OnImGuiRender();

        // Viewport
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

        DockingSpace::EndDocking();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        m_CameraController.OnEvent(e);
    }
}
