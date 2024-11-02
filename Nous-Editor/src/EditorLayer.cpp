#include "EditorLayer.h"

#include "Nous/Utils/PlatformUtils.h"
#include "Nous/Script/ScriptEngine.h"
#include "Nous/Renderer/Font.h"

#include "Nous/Asset/AssetManager.h"
#include "Nous/Asset/TextureImporter.h"
#include "Nous/Asset/SceneImporter.h"

#include "Panel/DockingSpace.h"
#include "Event/EditorEvent.h"

#include <imgui/imgui.h>

namespace Nous 
{
    static Ref<Font> s_Font;

    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
        EditorEventEmitter::AddObserver(this);

        s_Font = Font::GetDefault();
    }

    void EditorLayer::OnAttached()
    {
        NS_PROFILE_FUNCTION();

        m_IconPlay = TextureImporter::LoadTexture2D("resources/icons/PlayButton.png");
        m_IconSimulate = TextureImporter::LoadTexture2D("resources/icons/SimulateButton.png");
        m_IconStop = TextureImporter::LoadTexture2D("resources/icons/StopButton.png");
        m_IconPause = TextureImporter::LoadTexture2D("resources/icons/PauseButton.png");
        m_IconStep = TextureImporter::LoadTexture2D("resources/icons/StepButton.png");

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        //fbSpec.Samples = 4;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_EditorScene = CreateRef<Scene>();
        m_ActiveScene = m_EditorScene;


        // 根据命令行参数打开场景
        auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
        if (commandLineArgs.Count > 1)
        {
            auto projectFilePath = commandLineArgs[1];
            OpenProject(projectFilePath);
        }
        else
        {
            // TODO 选择一个文件夹
            // NewProject();
            // 如果没有打开的项目，直接关闭程序
            if (!OpenProject())
                Application::Get().Close();
        }

        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

        Renderer2D::SetLineWidth(4.0f);

        m_ViewportPanel.SetFramebuffer(m_Framebuffer);
        m_ViewportPanel.SetEditorCamera(&m_EditorCamera);
    }

    void EditorLayer::OnDetached()
    {
        NS_PROFILE_FUNCTION();
    }

    static float fps = 0.0f;

    void EditorLayer::OnUpdate(Timestep dt)
    {
        NS_PROFILE_FUNCTION();

        auto viewportSize = m_ViewportPanel.GetSize();
        m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

        fps = 1.0f / dt;

        // Resize
        auto spec = m_Framebuffer->GetSpecification();
        if (viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
            (spec.Width != (uint32_t)viewportSize.x ||
                spec.Height != (uint32_t)viewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
            m_EditorCamera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
        }

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
            case SceneState::Simulate:
            {
                m_EditorCamera.OnUpdate(dt);
                m_ActiveScene->OnUpdateSimulation(dt, m_EditorCamera);
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

        OnOverlayRender();

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
                // 打开项目
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                    OpenProject();

                ImGui::Separator();

                // 新建场景
                if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                    NewScene();

                // 保存
                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                    SaveScene();

                // 另存为
                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script", m_ActiveScene && !m_ActiveScene->IsRunning()))
            {
                if (ImGui::MenuItem("Reload", "Crtl+R"))
                    ScriptEngine::ReloadAssembly();
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        UI_Toolbar();

        // Hierarchy
        m_SceneHierarchyPanel.OnImGuiRender();

        // Resource
        m_ResourceBrowserPanel->OnImGuiRender();

        m_ConsolePanel.OnImGuiRender();

        // Viewport
        m_ViewportPanel.OnImGuiRender();

        ImGui::Begin("Settings");
        ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
#if 0
        ImGui::Image((ImTextureID)s_Font->GetAtlasTexture()->GetRendererID(), { 512, 512 }, { 0, 1 }, { 1, 0 });
#endif
        ImGui::End();

        // Properties
        ImGui::Begin("Stats");

#if 0
        std::string name = "None";
        if (m_ViewportPanel.GetHoveredEntity())
            name = m_ViewportPanel.GetHoveredEntity().GetComponent<CTag>().Tag;
        ImGui::Text("Hovered Entity: %s", name.c_str());
#endif

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

        bool toolbarEnabled = (bool)m_ActiveScene;

        ImVec4 tintColor = ImVec4(1, 1, 1, 1);
        if (!toolbarEnabled)
            tintColor.w = 0.5f;

        float size = ImGui::GetWindowHeight() - 4.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f)); // center
        
        bool hasPlayButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play;
        bool hasSimulateButton = m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate;
        bool hasPauseButton = m_SceneState != SceneState::Edit;

        // Play
        if (hasPlayButton)
        {
            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {
                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
                    OnScenePlay();
                else if (m_SceneState == SceneState::Play)
                    OnSceneStop();
            }
        }
        // Simulate
        if (hasSimulateButton)
        {
            if (hasPlayButton)
                ImGui::SameLine();

            Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;
            if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
            {
                if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
                    OnSceneSimulate();
                else if (m_SceneState == SceneState::Simulate)
                    OnSceneStop();
            }
        }

        // Pause
        if (hasPauseButton)
        {
            bool isPaused = m_ActiveScene->IsPaused();
            Ref<Texture2D> icon = isPaused ? m_IconPlay : m_IconPause;
            ImGui::SameLine();
            {
                if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                {
                    m_ActiveScene->SetPaused(!isPaused);
                }
            }

            // Step button
            if (isPaused)
            {
                ImGui::SameLine();
                {
                    Ref<Texture2D> icon = m_IconStep;
                    bool isPaused = m_ActiveScene->IsPaused();
                    if (ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
                    {
                        m_ActiveScene->Step();
                    }
                }
            }
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(NS_BIND_EVENT_FN(EditorLayer::OnKeyPressed));

        // 编辑状态才能移动编辑器摄像机
        if (m_SceneState == SceneState::Edit)
        {
            m_EditorCamera.OnEvent(e);
        }

        m_ViewportPanel.OnEvent(e);
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // 按下只响应一次
        if (e.IsRepeat())
            return false;

        // 快捷键
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
                    OpenProject();
                break;
            }
            case Key::S:
            {
                if (control)
                {
                    if (shift)
                        SaveSceneAs();
                    else
                        SaveScene();
                }
                break;
            }
            case Key::R:
            {
                if (control)
                {
                    if (m_ActiveScene && !m_ActiveScene->IsRunning())
                        ScriptEngine::ReloadAssembly();
                }
                break;
            }
            // Scene Commands
            case Key::D:
            {
                if (control)
                    OnDuplicateEntity();
                break;
            }
            case Key::Delete:
            {
                if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0 && m_ActiveScene)
                {
                    Entity selectedEntity = m_ActiveScene->GetSelectedEntity();
                    if (selectedEntity)
                    {
                        m_ActiveScene->SetSelectedEntity({});
                        m_ActiveScene->DestroyEntity(selectedEntity);
                    }
                }
                break;
            }
        }

        return false;
    }

    void EditorLayer::OnOverlayRender()
    {
        if (m_SceneState == SceneState::Play)
        {
            Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
            if (!camera)
                return;

            Renderer2D::BeginScene(camera.GetComponent<CCamera>().Camera, camera.GetComponent<CTransform>().GetTransform());
        }
        else
        {
            Renderer2D::BeginScene(m_EditorCamera);
        }


        if (m_ShowPhysicsColliders)
        {
            // Box Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<CTransform, CBoxCollider2D>();
                for (auto entity: view)
                {
                    auto [tc, bc2d] = view.get<CTransform, CBoxCollider2D>(entity);

                    //glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
                    glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
                        * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
                        * glm::scale(glm::mat4(1.0f), scale);

                    Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
                }
            }

            // Circle Colliders
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<CTransform, CCircleCollider2D>();
                for (auto entity : view)
                {
                    auto [tc, cc2d] = view.get<CTransform, CCircleCollider2D>(entity);

                    glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.01f);
                    glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::scale(glm::mat4(1.0f), scale);

                    Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.02f);
                }
            }
        }

        // 绘制选中实体的矩形边框
        if (Entity selectedEntity = m_ActiveScene->GetSelectedEntity()) {
            const CTransform& transform = selectedEntity.GetComponent<CTransform>();

            // Orange
            Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        }

        Renderer2D::EndScene();
    }

    void EditorLayer::NewProject()
    {
        Project::New();
    }

    bool EditorLayer::OpenProject()
    {
        std::string filepath = FileDialogs::OpenFile("Nous Project (*.nsproj)\0*.nsproj\0");
        if (filepath.empty())
            return false;

        OpenProject(filepath);
        return true;
    }

    void EditorLayer::OpenProject(const std::filesystem::path& path)
    {
        if (Project::Load(path))
        {
            ScriptEngine::Init();

            AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
            if (startScene)
                OpenScene(startScene);
            m_ResourceBrowserPanel = CreateScope<ResourceBrowserPanel>();
        }
    }

    void EditorLayer::SaveProject()
    {
        // Project::SaveActive();
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_EditorScene = CreateRef<Scene>();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_ViewportPanel.SetContext(m_ActiveScene);

        m_EditorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        // TODO
        /*std::string filepath = FileDialogs::OpenFile("Nous Scene (*nous)\0*.nous\0");
        if (!filepath.empty())
            OpenScene(filepath);*/
    }

    void EditorLayer::OpenScene(AssetHandle handle)
    {
        NS_CORE_ASSERT(handle);

        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(handle);
        Ref<Scene> newScene = Scene::Copy(readOnlyScene);

        m_EditorScene = newScene;
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_ViewportPanel.SetContext(m_EditorScene);

        m_ActiveScene = m_EditorScene;
        m_EditorScenePath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle);

        m_EditorCamera.Reset();
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
        {
            SerializeScene(m_EditorScene, m_EditorScenePath);

            // 保存后马上作为资源加载
            Project::GetActive()->GetEditorAssetManager()->ReloadAsset(m_EditorScene->Handle);

            // 更新资源面板
            m_ResourceBrowserPanel->RefreshAssetTree();
        }
        else
        {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::filesystem::path filepath = FileDialogs::SaveFile("Nous Scene (*Nous)\0*.nous\0");
        filepath = std::filesystem::relative(filepath, Project::GetAssetsDirectory());
        std::string pathString = filepath.generic_string();

        if (!pathString.empty())
        {
            SerializeScene(m_ActiveScene, pathString);
            m_EditorScenePath = pathString;

            // 保存后马上作为资源加载
            Project::GetActive()->GetEditorAssetManager()->ImportAsset(filepath);

            // 更新资源面板
            m_ResourceBrowserPanel->RefreshAssetTree();
        }
    }

    void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
    {
        SceneImporter::SaveScene(scene, path);
    }

    void EditorLayer::OnEditorEvent(EditorEvent& e)
    {
        EditorEventDispatcher dispatcher(e);
        dispatcher.Dispatch<OpenSceneEvent>(NS_BIND_EVENT_FN(EditorLayer::OnOpenScene));
        dispatcher.Dispatch<AssetFileDoubleClickEvent>(NS_BIND_EVENT_FN(EditorLayer::OnAssetFileDoubleClick));
    }

    void EditorLayer::OnOpenScene(OpenSceneEvent& e)
    {
        if (e.Handle)
            OpenScene(e.Handle);
    }

    void EditorLayer::OnAssetFileDoubleClick(AssetFileDoubleClickEvent& e)
    {
        if (AssetManager::GetAssetType(e.Handle) == AssetType::Scene)
        {
            OpenScene(e.Handle);
        }
    }

    void EditorLayer::OnScenePlay()
    {
        if (m_SceneState == SceneState::Simulate)
            OnSceneStop();

        ConsoleClearEvent e;
        EditorEventEmitter::Emit(e);

        m_SceneState = SceneState::Play;

        m_ActiveScene = Scene::Copy(m_EditorScene); // 复制一个副本来运行
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_ViewportPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneSimulate()
    {
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        m_SceneState = SceneState::Simulate;

        m_ActiveScene = Scene::Copy(m_EditorScene);
        m_ActiveScene->OnSimulationStart();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneStop()
    {
        NS_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        m_SceneState = SceneState::Edit;

        m_ActiveScene->OnRuntimeStop();
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_ViewportPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDuplicateEntity()
    {
        if (m_SceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_ActiveScene->GetSelectedEntity();
        if (selectedEntity)
            m_EditorScene->DuplicateEntity(selectedEntity);
    }
}
