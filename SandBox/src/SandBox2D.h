#pragma once

#include "Nous.h"
#include "ViewportPanel.h"

namespace Nous
{
    class SandBox2D : public Layer
    {
    public:
        SandBox2D();
        virtual ~SandBox2D() = default;

        virtual void OnAttached() override;
        virtual void OnDetached() override;

        virtual void OnUpdate(Timestep dt) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;
    private:
        bool OpenProject();
        void OpenProject(const std::filesystem::path& path);

        void OpenScene();
        void OpenScene(AssetHandle handle);

        void OnScenePlay();

    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
        std::filesystem::path m_EditorScenePath;

        EditorCamera m_EditorCamera;

        // ��崰��
        ViewportPanel m_ViewportPanel;

        bool m_ShowPhysicsColliders = false;

        enum class SceneState
        {
            Edit = 0, Play = 1, Simulate = 2 // ģ�⣺�ñ༭�������������ʹ�ó��������
        };
        SceneState m_SceneState = SceneState::Play;
    };
}
