#pragma once

#include "Nous/Renderer/Framebuffer.h"

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

        void OnImGuiRender();
    private:
        Ref<Framebuffer> m_Framebuffer;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
    };

}

