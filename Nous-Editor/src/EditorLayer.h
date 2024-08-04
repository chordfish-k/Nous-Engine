#pragma once

#include "Nous.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Panel/ViewportPanel.h"

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
        Ref<VertexArray> m_SquareVA;
        Ref<Shader> m_FlatColorShader;
        Ref<Framebuffer> m_Framebuffer;

        Ref<Scene> m_ActiveScene;
        Entity m_SquareEntity;
        Entity m_CameraEntity;

        Ref<Texture2D> m_MarioTexture, m_CheckerboardTexture;

        glm::vec4 m_SquareColor = {0.2f, 0.3f, 0.8f, 1.0f};

        // 面板窗口
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ViewportPanel m_ViewportPanel;
    };
}