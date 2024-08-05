#include "EditorLayer.h"

#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

#include "Panel/DockingSpace.h"

#include "Nous/Utils/PlatformUtils.h"

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
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        // Entity
/*
        m_SquareEntity = m_ActiveScene->CreateEntity("Green Square");
        m_SquareEntity.AddComponent<CSpriteRenderer>(glm::vec4{0.3f, 0.7f, 0.2f, 1.0f});

        auto redSquare = m_ActiveScene->CreateEntity("Red Square");
        redSquare.AddComponent<CSpriteRenderer>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

        m_CameraEntity = m_ActiveScene->CreateEntity("Ortho Camera", {0.0f, 0.0f, 1.0f});
        m_CameraEntity.AddComponent<CCamera>();

        auto cameraEntity2 = m_ActiveScene->CreateEntity("Persp Camera", {0.0f, 0.0f, 1.0f});
        auto&cc = cameraEntity2.AddComponent<CCamera>();
        cc.Primary = false;
        cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);

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
                auto& pos = GetComponent<CTransform>().Translation;
                float speed = 5.0f;
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
*/
        m_SceneHierarchyPanel.SetContent(m_ActiveScene);
        m_ViewportPanel.SetFramebuffer(m_Framebuffer);
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
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        NS_CORE_WARN("YEs");
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

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        auto viewportSize = m_ViewportPanel.GetSize();
        m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
        m_SceneHierarchyPanel.SetContent(m_ActiveScene);
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
