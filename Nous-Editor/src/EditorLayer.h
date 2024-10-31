#pragma once

#include "Nous.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ResourceBrowserPanel.h"
#include "Panel/ViewportPanel.h"
#include "Panel/ConsolePanel.h"

#include "Nous/Renderer/EditorCamera.h"

#include "Event/Observer.h"

namespace Nous {

    class EditorLayer : public Layer, public Observer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttached() override;
        virtual void OnDetached() override;

        virtual void OnUpdate(Timestep dt) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

        virtual void OnEditorEvent(EditorEvent& e) override;
    private:
        bool OnKeyPressed(KeyPressedEvent& e);

        void OnOpenScene(OpenSceneEvent& e);
        void OnAssetFileDoubleClick(AssetFileDoubleClickEvent& e);
        
        void OnOverlayRender(); // 覆盖层绘制

        void NewProject();
        bool OpenProject();
        void OpenProject(const std::filesystem::path& path);
        void SaveProject();

        void NewScene();
        void OpenScene();
        void OpenScene(AssetHandle handle);
        void SaveScene();
        void SaveSceneAs();

        void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();
        void OnScenePause();

        void OnDuplicateEntity();

        // UI Panels
        void UI_Toolbar();
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
        std::filesystem::path m_EditorScenePath;
        
        EditorCamera m_EditorCamera;

        // 面板窗口
        SceneHierarchyPanel m_SceneHierarchyPanel;
        Scope<ResourceBrowserPanel> m_ResourceBrowserPanel;
        Console m_ConsolePanel;
        ViewportPanel m_ViewportPanel;

        bool m_ShowPhysicsColliders = false;

        enum class SceneState
        {
            Edit = 0, Play = 1, Simulate = 2 // 模拟：用编辑器的摄像机，不使用场景摄像机
        };
        SceneState m_SceneState = SceneState::Edit;

        // Editor resources
        Ref<Texture2D> m_IconPlay, m_IconSimulate, m_IconStop, m_IconPause, m_IconStep;
    };
}