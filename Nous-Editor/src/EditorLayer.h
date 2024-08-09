#pragma once

#include "Nous.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ResourceBrowserPanel.h"
#include "Panel/ViewportPanel.h"

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

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();

        void OnScenePlay();
        void OnSceneStop();

        // UI Panels
        void UI_Toolbar();
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;

        fs::path m_EditorScenePath;
        EditorCamera m_EditorCamera;

        // 面板窗口
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ResourceBrowserPanel m_ResourceBrowserPanel;
        ViewportPanel m_ViewportPanel;

        enum class SceneState
        {
            Edit = 0, Play = 1
        };
        SceneState m_SceneState = SceneState::Edit;

        // Editor resources
        Ref<Texture2D> m_IconPlay, m_IconStop;
    };
}