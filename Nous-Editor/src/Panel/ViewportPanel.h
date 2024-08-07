#pragma once

#include "Nous/Renderer/Framebuffer.h"
#include "Nous/Renderer/EditorCamera.h"
#include "Nous/Scene/Scene.h"

#include <glm/glm.hpp>

namespace Nous
{

    class ViewportPanel
    {
    public:
        ViewportPanel() = default;
        ViewportPanel(const Ref<Framebuffer>& framebuffer);

        void SetFramebuffer(const Ref<Framebuffer>& framebuffer);
        // 包含标签栏，整个窗口的大小
        const glm::vec2& GetSize() const { return m_ViewportSize; }
        // 不包含标签栏，内容的大小
        const glm::vec2& GetContentSize() const { return m_ViewportContentSize; }
        const glm::vec2& GetMinBound() const { return m_ViewportBounds[0]; }
        const glm::vec2& GetMaxBound() const { return m_ViewportBounds[1]; }

        void SetContent(const Ref<Scene>& scene);
        void SetEditorCamera(const Ref<EditorCamera>& camera);

        const Ref<EditorCamera>& GetEditorCamera() const { return m_EditorCamera; }

        void OnImGuiRender();
        void SetGizmoType(int type) { m_GizmoType = type;};
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_Context;
        Ref<EditorCamera> m_EditorCamera;

        int m_GizmoType = -1; // no Gizmo
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportContentSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];
    };

}

