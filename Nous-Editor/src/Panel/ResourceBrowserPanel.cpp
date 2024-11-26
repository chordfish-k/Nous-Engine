#include "pch.h"
#include "ResourceBrowserPanel.h"

#include "Nous/Project/Project.h"
#include "Nous/Asset/TextureImporter.h"

#include "Nous/Event/AppEvent.h"

#include <imgui.h>

namespace Nous 
{
    ResourceBrowserPanel::ResourceBrowserPanel(Ref<Project> project)
        : m_Project(project), m_ThumbnailCache(CreateRef<ThumbnailCache>(project)), m_BaseDirectory(Project::GetActiveAssetDirectory()),  m_CurrentDirectory(m_BaseDirectory)
    {
        m_TreeNodes.push_back(TreeNode(".", 0));

        m_DirectoryIcon = TextureImporter::LoadTexture2D("resources/icons/ResourceBrowser/DirectoryIcon.png");
        m_FileIcon = TextureImporter::LoadTexture2D("resources/icons/ResourceBrowser/FileIcon.png");
    
        RefreshAssetTree();

        m_Mode = Mode::FileSystem;
    }

    void ResourceBrowserPanel::OnImGuiRender()
    {
        NS_PROFILE_FUNCTION();

        ImGui::Begin("Resources");

        ImGui::Text("Mode:");
        ImGui::SameLine();
        const char* label = m_Mode == Mode::Asset ? "Asset" : "File";
        if (ImGui::Button(label))
        {
            m_Mode = m_Mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;
        }

        if (m_CurrentDirectory != m_BaseDirectory)
        {
            ImGui::SameLine();
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }


        static float padding = 16.0f;
        static float thumbnailSize = 100.0f; // 缩略图尺寸
        float cellSize = thumbnailSize + padding;

        const float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int) (panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::SameLine();
        ImGui::SliderFloat("Icon Size", &thumbnailSize, 16, 512);

        ImGui::BeginChild("Icons", {0, 0}, false);

        ImGui::Columns(columnCount, 0, false);

        if (m_Mode == Mode::Asset)
        {
            TreeNode* node = &m_TreeNodes[0];

            for (const auto& p : m_CurrentDirectory)
            {
                // 如果只有一级目录
                if (0 == m_CurrentDirectory.compare(Project::GetActiveAssetDirectory() / node->Path))
                    break;

                if (node->Children.find(p) != node->Children.end())
                {
                    node = &m_TreeNodes[node->Children[p]];
                    continue;
                }
                else
                {
                    // 到达无效的路径，设置回m_TreeNodes[0]
                    node = &m_TreeNodes[0];
                }
            }

            for (const auto& [item, treeNodeIndex] : node->Children)
            {
                const std::string itemStr = item.generic_string();
                const auto path = m_CurrentDirectory / item;
                const bool isDirectory = std::filesystem::is_directory(path);

                // 缩略图 Thumbnail
                Ref<Texture2D> thumbnail = m_DirectoryIcon;
                if (!isDirectory)
                {
                    thumbnail = m_ThumbnailCache->GetOrCreateThumbnail(path);
                    if (!thumbnail)
                        thumbnail = m_FileIcon;
                }

                ImGui::PushID(itemStr.c_str());
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton((ImTextureID)thumbnail->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                bool isDelete = false;
                if (!isDirectory && ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        Project::GetActive()->GetEditorAssetManager()->RemoveAsset(m_TreeNodes[treeNodeIndex].Handle);
                        ClearAssetTree();
                        RefreshAssetTree();

                        isDelete = true;
                    }
                    ImGui::EndPopup();
                }

                if (!isDirectory && ImGui::BeginDragDropSource())
                {
                    AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
                    ImGui::Text(itemStr.c_str());
                    ImGui::SetDragDropPayload("RESOURCE_BROWSER_ITEM", &handle, sizeof(handle));
                    ImGui::EndDragDropSource();
                }

                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (isDirectory)
                        m_CurrentDirectory /= item.filename();
                    else
                    {
                        // 发送资源双击事件
                        AssetFileDoubleClickEvent event{ m_TreeNodes[treeNodeIndex].Handle };
                        AppEventEmitter::Emit(event);
                    }
                }

                ImGui::TextWrapped("%s", itemStr.c_str()); // 显示在底部的文件名
                ImGui::NextColumn();
                ImGui::PopID();

                if (isDelete)
                    break;
            }
        }
        else
        {
            std::vector<std::filesystem::directory_entry> fileEntries;

            // 文件夹
            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
            {
                const auto& path = directoryEntry.path();
                const std::string &fileNameString = path.filename().string();

                const bool isDirectory = directoryEntry.is_directory();

                if (isDirectory)
                {
                    // 缩略图 Thumbnail
                    const Ref<Texture2D> thumbnail = m_DirectoryIcon;
                
                    ImGui::PushID(fileNameString.c_str());
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    
                    ImGui::ImageButton((ImTextureID)(uint64_t)thumbnail->GetRendererID(), { thumbnailSize, thumbnailSize }, {0, 1}, {1, 0});

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        m_CurrentDirectory /= path.filename();
                    }
                    
                    ImGui::PopStyleColor();
                    ImGui::TextWrapped("%s", fileNameString.c_str()); // 显示在底部的文件名
                    ImGui::NextColumn();
                    ImGui::PopID();
                }
                else
                {
                    fileEntries.push_back(directoryEntry);
                }
            }

            for (auto& directoryEntry : fileEntries)
            {
                NS_PROFILE_SCOPE("FILE");

                const auto& path = directoryEntry.path();
                const std::string &fileNameString = path.filename().string();

                // 缩略图 Thumbnail
                Ref<Texture2D> thumbnail = m_FileIcon;

                thumbnail = m_ThumbnailCache->GetOrCreateThumbnail(path);
                if (!thumbnail)
                    thumbnail = m_FileIcon;

                ImGui::PushID(fileNameString.c_str());
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton((ImTextureID)(uint64_t)thumbnail->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Import"))
                    {
                        Project::GetActive()->GetEditorAssetManager()->ImportAsset(path);
                        RefreshAssetTree();
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor();
                ImGui::TextWrapped("%s", fileNameString.c_str()); // 显示在底部的文件名
                ImGui::NextColumn();
                ImGui::PopID();
            }
        }

        ImGui::Columns(1);

        ImGui::EndChild();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TRANSFORM_NODE"))
            {
                UUID uuid = *(UUID*)payload->Data;

                SavePrefabEvent event{ uuid , m_CurrentDirectory };
                AppEventEmitter::Emit(event);
            }
            ImGui::EndDragDropTarget();
        }
        
        ImGui::End();
    }

    void ResourceBrowserPanel::ClearAssetTree()
    {
        m_TreeNodes.clear();
        m_TreeNodes.push_back(TreeNode(".", 0));
    }

    void ResourceBrowserPanel::RefreshAssetTree()
    {
        const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
        for (const auto& [handle, metadata] : assetRegistry)
        {
            uint32_t currentNodeIndex = 0;

            for (const auto& p : metadata.FilePath)
            {
                std::string key = std::filesystem::absolute(p).string();
                auto it = m_TreeNodes[currentNodeIndex].Children.find((p.generic_string()));
                if (it != m_TreeNodes[currentNodeIndex].Children.end())
                {
                    currentNodeIndex = it->second;
                }
                else
                {
                    // 添加节点
                    TreeNode newNode(p, handle);
                    newNode.Parent = currentNodeIndex;
                    m_TreeNodes.push_back(newNode);

                    m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
                    currentNodeIndex = m_TreeNodes.size() - 1;
                }
            }
        }
    }
}