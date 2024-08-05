#include "pch.h"
#include "ViewportPanel.h"

#include "Nous/Core/Application.h"

#include <imgui.h>

namespace Nous {


    ViewportPanel::ViewportPanel(const Ref<Framebuffer>& framebuffer)
    {
        SetFramebuffer(framebuffer);
    }

    void ViewportPanel::SetFramebuffer(const Ref<Framebuffer>& framebuffer)
    {
        m_Framebuffer = framebuffer;
    }

    void ViewportPanel::OnImGuiRender()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::Begin("Viewport");

        // TODO 修复焦点不在Viewport不能用快捷键的bug
        Application::Get().GetImGuiLayer()->SetBlockEvent(!ImGui::IsWindowFocused() || !ImGui::IsWindowHovered());

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*) textureID, {m_ViewportSize.x, m_ViewportSize.y}, {0, 1}, {1, 0});

        ImGui::End();
        ImGui::PopStyleVar();
    }
}