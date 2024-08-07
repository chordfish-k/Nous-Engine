#include "EditorLayer.h"

#include "Nous/Utils/PlatformUtils.h"
#include "Panel/DockingSpace.h"

#include <imgui/imgui.h>
#include <ImGuizmo.h>

namespace Nous {

    EditorLayer::EditorLayer()
        : Layer("SandBox2D")
    {
    }

    void EditorLayer::OnAttached()
    {
        NS_PROFILE_FUNCTION();

        m_MarioTexture = Texture2D::Create("assets/textures/Mario.png");
        m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");

        FramebufferSpecification fbSpec = {};
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth};
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();
        m_EditorCamera = CreateRef<EditorCamera>(30.0f, 1.778f, 0.1f, 1000.0f);

        m_ViewportPanel.SetFramebuffer(m_Framebuffer);
        m_ViewportPanel.SetEditorCamera(m_EditorCamera);

        m_SceneHierarchyPanel.SetContent(m_ActiveScene);
        m_ViewportPanel.SetContent(m_ActiveScene);
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
        auto viewportSize = m_ViewportPanel.GetSize();
        if (viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
            (spec.Width != (uint32_t) viewportSize.x ||
             spec.Height != (uint32_t) viewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t) viewportSize.x, (uint32_t) viewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t) viewportSize.x, (uint32_t) viewportSize.y);
        }

        // Update
        m_EditorCamera->OnUpdate(dt);

        // Render
        Renderer2D::ResetStats();
        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear(); // 会把entity ID 附件也统一设置成这个值

        // 设置 entity ID 附件的值为 -1
        m_Framebuffer->ClearAttachment(1, -1);

        // Update Scene
        m_ActiveScene->OnUpdateEditor(dt, *m_EditorCamera);

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportPanel.GetMinBound().x;
        my -= m_ViewportPanel.GetMinBound().y;
        // 不包含标签栏
        auto viewportContentSize = m_ViewportPanel.GetContentSize();
        my = viewportContentSize.y - my;
        int mouseX = (int) mx;
        int mouseY = (int) my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportContentSize.x && mouseY < (int)viewportContentSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
        }


        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
        NS_PROFILE_FUNCTION();

        DockingSpace::BeginDocking();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();

                // 保存
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                    SaveScene();

                // 另存为
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", ""))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // Properties
        ImGui::Begin("Stats");
        std::string name = "None";
        if (m_HoveredEntity)
            name = m_HoveredEntity.GetComponent<CTag>().Tag;
        ImGui::Text("Hovered Entity: %s", name.c_str());

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
        m_ViewportPanel.OnImGuiRender();

        DockingSpace::EndDocking();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(NS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(NS_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));

        m_EditorCamera->OnEvent(e);
        m_ViewportPanel.OnEvent(e);
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // 快捷键
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (e.GetKeyCode())
        {
            case Key::N:
            {
                if (control)
                    NewScene();
                break;
            }
            case Key::O:
            {
                if (control)
                    OpenScene();
                break;
            }
            case Key::S:
            {
                if (control)
                {
                    if ( shift || m_EditorScenePath.empty())
                        SaveSceneAs();
                    else
                        SaveScene();
                }
                break;
            }
            default:
                return false;
        }
        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportPanel.IsHovered() && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_ActiveScene->SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        auto viewportSize = m_ViewportPanel.GetSize();
        m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
        m_SceneHierarchyPanel.SetContent(m_ActiveScene);
        m_ViewportPanel.SetContent(m_ActiveScene);
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Scene File (*scn)\0*.scn\0");
        if (!filepath.empty())
        {
            m_ActiveScene = CreateRef<Scene>();
            auto viewportSize = m_ViewportPanel.GetSize();
            m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
            m_SceneHierarchyPanel.SetContent(m_ActiveScene);
            m_ViewportPanel.SetContent(m_ActiveScene);

            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(filepath);

            m_EditorScenePath = filepath;
        }
    }

    void EditorLayer::SaveScene()
    {
        SceneSerializer serializer(m_ActiveScene);
        serializer.Serialize(m_EditorScenePath.string());
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Scene File (*scn)\0*.scn\0");

        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);

            m_EditorScenePath = filepath;
        }
    }
}
