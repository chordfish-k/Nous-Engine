#include "ResourceBrowserPanel.h"

#include <imgui.h>

namespace Nous {

    static const std::filesystem::path s_AssetsPath = "assets";

    ResourceBrowserPanel::ResourceBrowserPanel()
        : m_CurrentDirectory(s_AssetsPath)
    {
    }

    void ResourceBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Resources");

        for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            ImGui::Text("%s", p.path().string().c_str());
            p.path();
        }

        ImGui::End();
    }
}