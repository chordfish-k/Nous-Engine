#pragma once

#include "Nous/Renderer/Framebuffer.h"
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
        const glm::vec2& GetSize() const { return m_ViewportSize; }

        void SetContent(const Ref <Scene>& scene);

        void OnImGuiRender();
        void SetGizmoType(int type) { m_GizmoType = type;};
    private:
        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_Context;

        int m_GizmoType = -1; // no Gizmo

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
    };

}

