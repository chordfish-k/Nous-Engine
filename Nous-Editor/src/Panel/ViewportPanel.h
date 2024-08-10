#pragma once

#include "Nous/Renderer/Framebuffer.h"
#include "Nous/Renderer/EditorCamera.h"
#include "Nous/Scene/Scene.h"
#include "Nous/Scene/Entity.h"
#include "Nous/Event/KeyEvent.h"
#include "Nous/Renderer/Texture.h"

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

        void SetContext(const Ref<Scene>& scene);
        void SetEditorCamera(EditorCamera* camera);

        EditorCamera* GetEditorCamera() const { return m_EditorCamera; }

        void CheckAndResize();
        void CheckHoveredEntity();
        void OnImGuiRender();
        void OnEvent(Event& event);

        void SetGizmoType(int type) { m_GizmoType = type;}

        bool IsHovered() const { return m_ViewportHovered; };
        bool IsFocused() const { return m_ViewportFocused; };

        Entity GetHoveredEntity() { return m_HoveredEntity; }
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_Context;
        EditorCamera* m_EditorCamera;

        int m_GizmoType = -1; // no Gizmo
        bool m_ShowGizmo = false;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportContentSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];

        Entity m_HoveredEntity {entt::null, nullptr};

        bool m_ViewportHovered = false;
        bool m_ViewportFocused = false;
    };

}

