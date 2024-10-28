﻿#include "pch.h"
#include "ResourceBrowserPanel.h"

#include "Nous/Project/Project.h"

#include <imgui.h>

namespace Nous 
{
    ResourceBrowserPanel::ResourceBrowserPanel()
        : m_BaseDirectory(Project::GetAssetsDirectory()), m_CurrentDirectory(m_BaseDirectory)
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/ResourceBrowser/DirectoryIcon.png");
        m_FileIcon = Texture2D::Create("resources/icons/ResourceBrowser/FileIcon.png");
    }

    void ResourceBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Resources");

        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }


        static float padding = 16.0f;
        static float thumbnailSize = 128.0f; // 缩略图尺寸
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int) (panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::SameLine();
        ImGui::SliderFloat("Icon Size", &thumbnailSize, 16, 512);

        ImGui::Columns(columnCount, 0, false);


        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            std::string fileNameString = path.filename().string();

            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
            ImGui::PushID(fileNameString.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton((ImTextureID)(uint64_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource())
            {
                auto relativePath = std::filesystem::relative(path);
                const wchar_t* itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("RESOURCE_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }

            ImGui::TextWrapped("%s", fileNameString.c_str()); // 显示在底部的文件名
            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);


        ImGui::End();
    }
}