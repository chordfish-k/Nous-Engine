#include "ResourceBrowserPanel.h"

#include <imgui.h>

namespace Nous {

    static const std::filesystem::path s_AssetPath = "assets";

    ResourceBrowserPanel::ResourceBrowserPanel()
        : m_CurrentDirectory(s_AssetPath)
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/DirectoryIcon.png");
        m_FileIcon = Texture2D::Create("resources/icons/FileIcon.png");
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


        static float padding = 16.0f;
        static float thumbnailSize = 128.0f; // 缩略图尺寸
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvailWidth();
        int columnCount = (int) (panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::SameLine();
        ImGui::SliderFloat("Icon Size", &thumbnailSize, 16, 512);

        ImGui::Columns(columnCount, 0, false);


        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, s_AssetPath);
            auto fileNameString = relativePath.filename().string();

            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
            ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, {0, 1}, {1, 0});

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }

            ImGui::TextWrapped("%s", fileNameString.c_str()); // 显示在底部的文件名
            ImGui::NextColumn();
        }

        ImGui::Columns(1);


        ImGui::End();
    }
}