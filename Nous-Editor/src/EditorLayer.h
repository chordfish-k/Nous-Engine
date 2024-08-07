#pragma once

#include "Nous.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ViewportPanel.h"

#include "Nous/Renderer/EditorCamera.h"

namespace Nous {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttached() override;
        virtual void OnDetached() override;

        virtual void OnUpdate(Timestep dt) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;
    private:
        bool OnKeyPressed(KeyPressedEvent& e);

        void NewScene();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
    private:
        Ref<EditorCamera> m_EditorCamera;
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;

        fs::path m_EditorScenePath;

        Ref<Texture2D> m_MarioTexture, m_CheckerboardTexture;

        // 面板窗口
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ViewportPanel m_ViewportPanel;
    };
}