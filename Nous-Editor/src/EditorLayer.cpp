#include "EditorLayer.h"

#include "Nous/Utils/PlatformUtils.h"
#include "Panel/DockingSpace.h"
#include "Event/EditorEvent.h"

#include <imgui/imgui.h>

namespace Nous {

    extern const std::filesystem::path g_AssetPath;

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
        EditorEventRepeater::AddObserver(this);
    }

    void EditorLayer::OnAttached()
    {
        NS_PROFILE_FUNCTION();

        m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
        m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");

        FramebufferSpecification fbSpec = {};
        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA8,            // 颜色缓冲
            FramebufferTextureFormat::RED_INTEGER,      // 实体ID
            FramebufferTextureFormat::Depth           // 深度缓冲
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        // 根据命令行参数打开场景
        auto commandLineArgs = Application::Get().GetCommandLineArgs();
        if (commandLineArgs.Count > 1)
        {
            auto sceneFilePath = commandLineArgs[1];
            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(sceneFilePath);
        }

        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

        m_ViewportPanel.SetFramebuffer(m_Framebuffer);
        m_ViewportPanel.SetEditorCamera(&m_EditorCamera);

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_ViewportPanel.SetContext(m_ActiveScene);
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
        m_ViewportPanel.CheckAndResize();

        // Update
        m_EditorCamera.OnUpdate(dt);

        m_Framebuffer->Bind();

        // Clear
        Renderer2D::ResetStats();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear(); // 会把entity ID 附件也统一设置成这个值
        m_Framebuffer->ClearAttachment(1, -1); // 设置 entity ID 附件的值为 -1

        // Render
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                m_EditorCamera.OnUpdate(dt);
                m_ActiveScene->OnUpdateEditor(dt, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(dt);
                break;
            }
        }

        // Postprocess
        m_ViewportPanel.CheckHoveredEntity();

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
        NS_PROFILE_FUNCTION();

        DockingSpace::BeginDocking();

        // Menu
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

        UI_Toolbar();

        // Hierarchy
        m_SceneHierarchyPanel.OnImGuiRender();

        // Resource
        m_ResourceBrowserPanel.OnImGuiRender();

        // Viewport
        m_ViewportPanel.OnImGuiRender();

        // Properties
        ImGui::Begin("Stats");
        std::string name = "None";
        if (m_ViewportPanel.GetHoveredEntity())
            name = m_ViewportPanel.GetHoveredEntity().GetComponent<CTag>().Tag;
        ImGui::Text("Hovered Entity: %s", name.c_str());

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quads: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::Text("FPS: %.2f", fps);
        ImGui::End();

        DockingSpace::EndDocking();
    }

    void EditorLayer::UI_Toolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float size = ImGui::GetWindowHeight() - 4.0f;
        Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
        if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            if (m_SceneState == SceneState::Edit)
                OnScenePlay();
            else if (m_SceneState == SceneState::Play)
                OnSceneStop();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(NS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));

        m_EditorCamera.OnEvent(e);
        m_ViewportPanel.OnEvent(e);
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // 快捷键
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        // Save & Load & Open
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

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        auto viewportSize = m_ViewportPanel.GetSize();
        m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_ViewportPanel.SetContext(m_ActiveScene);

        m_EditorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Scene File (*scn)\0*.scn\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        Ref<Scene> newScene = CreateRef<Scene>();
        auto viewportSize = m_ViewportPanel.GetSize();

        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string()))
        {
            m_ActiveScene = newScene;
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_ViewportPanel.SetContext(m_ActiveScene);

            m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
            m_EditorScenePath = path;

            m_EditorCamera.Reset();
        }
    }

    void EditorLayer::SaveScene()
    {
        SceneSerializer serializer(m_ActiveScene);
        if (!m_EditorScenePath.empty())
            serializer.Serialize(m_EditorScenePath.string());
        else
            SaveSceneAs();
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

    void EditorLayer::OnEditorEvent(EditorEvent& e)
    {
        EditorEventDispatcher dispatcher(e);
        dispatcher.Dispatch<OpenSceneEvent>(NS_BIND_EVENT_FN(EditorLayer::OnOpenScene));
    }

    void EditorLayer::OnOpenScene(OpenSceneEvent& e)
    {
        if (e.FilePath.empty())
            OpenScene();
        else
            OpenScene(e.FilePath);
    }

    void EditorLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;
        m_ActiveScene->OnRuntimeStart();
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
        m_ActiveScene->OnRuntimeStop();
    }
}
