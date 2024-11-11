#include "SandBox2D.h"

#include "Nous/Asset/AssetManager.h"
#include "Nous/Utils/PlatformUtils.h"
#include "Nous/Script/ScriptEngine.h"
#include "DockingSpace.h"

#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

namespace Nous
{
    static Ref<Font> s_Font;

    SandBox2D::SandBox2D()
        : Layer(nullptr, "SandBox2D")
    {
        s_Font = Font::GetDefault();
    }

    SandBox2D::SandBox2D(Application* application, const ApplicationSpecification& spec)
        : Layer(application, "SandBox2D")
    {
    }

    void SandBox2D::OnAttached()
    {
        NS_PROFILE_FUNCTION();

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

    void SandBox2D::OnDetached()
    {
        NS_PROFILE_FUNCTION();
    }


    void SandBox2D::OnUpdate(Timestep dt)
    {
        NS_PROFILE_FUNCTION();

        auto viewportSize = m_ViewportPanel.GetSize();
        m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

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
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear(); // 会把entity ID 附件也统一设置成这个值
        m_Framebuffer->ClearAttachment(1, -1); // 设置 entity ID 附件的值为 -1

        // Render
        

        m_ActiveScene->OnUpdateRuntime(dt);

        // Postprocess
        m_ViewportPanel.CheckHoveredEntity();

        m_Framebuffer->Unbind();
    }

    void SandBox2D::OnImGuiRender()
    {
        NS_PROFILE_FUNCTION();

        DockingSpace::BeginDocking();
        // Viewport
        m_ViewportPanel.OnImGuiRender();

        ImGui::Begin("Settings");

        bool vsync = Application::Get().GetWindow().IsVSync();
        if (ImGui::Checkbox("vsync", &vsync))
        {
            Application::Get().GetWindow().SetVSync(vsync);
        }
        ImGui::End();

        DockingSpace::EndDocking();
    }


    void SandBox2D::OnEvent(Event& e)
    {
        m_ViewportPanel.OnEvent(e);
    }

    bool SandBox2D::OpenProject()
    {
        std::string filepath = FileDialogs::OpenFile("Nous Project (*.nsproj)\0*.nsproj\0");
        if (filepath.empty())
            return false;

        OpenProject(filepath);
        return true;
    }

    void SandBox2D::OpenProject(const std::filesystem::path& path)
    {
        if (Project::Load(path))
        {
            ScriptEngine::InitApp();

            AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
            if (startScene)
            {
                OpenScene(startScene);
            }
        }
    }


    void SandBox2D::OpenScene()
    {
        // TODO
        /*std::string filepath = FileDialogs::OpenFile("Nous Scene (*nous)\0*.nous\0");
        if (!filepath.empty())
            OpenScene(filepath);*/
    }

    void SandBox2D::OpenScene(AssetHandle handle)
    {
        NS_CORE_ASSERT(handle);


        Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(handle);
        Ref<Scene> newScene = Scene::Copy(readOnlyScene);

        m_EditorScene = newScene;
        m_ViewportPanel.SetContext(m_EditorScene);

        m_ActiveScene = m_EditorScene;
        m_EditorScenePath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle);

        m_EditorCamera.Reset();

        OnScenePlay();
    }


    void SandBox2D::OnScenePlay()
    {
        m_SceneState = SceneState::Play;

        m_ActiveScene = Scene::Copy(m_EditorScene); // 复制一个副本来运行
        m_ActiveScene->OnRuntimeStart();

        m_ViewportPanel.SetContext(m_ActiveScene);
    }
}
