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
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        fs::path m_EditorScenePath;
        Ref<EditorCamera> m_EditorCamera;
        Entity m_HoveredEntity;

        Ref<Texture2D> m_MarioTexture, m_CheckerboardTexture;

        glm::vec4 m_SquareColor = {0.2f, 0.3f, 0.8f, 1.0f};

        // 面板窗口
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ViewportPanel m_ViewportPanel;
    };
}