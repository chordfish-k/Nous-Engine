#include "ResourceBrowserPanel.h"

#include <imgui.h>

namespace Nous {

    static const std::filesystem::path s_AssetPath = "assets";

    ResourceBrowserPanel::ResourceBrowserPanel()
        : m_CurrentDirectory(s_AssetPath)
    {
    }

    void ResourceBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Resources");

        if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, s_AssetPath);
            auto fileNameString = relativePath.filename().string();
            if (directoryEntry.is_directory())
            {
                if (ImGui::Button(fileNameString.c_str()))
                {
                    m_CurrentDirectory /= path.filename();
                }
            }
            else
            {
                if (ImGui::Button(fileNameString.c_str()))
                {
                }
            }
        }

        ImGui::End();
    }
}