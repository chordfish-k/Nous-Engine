#include "pch.h"
#include "UI.h"

#include <misc/cpp/imgui_stdlib.h>

namespace Nous::EUI
{
    bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        float width = std::round(ImGui::GetContentRegionAvail().x);

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        float btnWidth = lineHeight + 3.0f;
        float xW = std::round(width / 3.0f) - btnWidth;

        ImVec2 buttonSize = { btnWidth, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            values.x = resetValue;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset");
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(xW);
        bool status = ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            values.y = resetValue;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset");
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(xW);
        status = ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            values.z = resetValue;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset");
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(width - xW * 2 - btnWidth * 3);
        status = ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        float width = std::round(ImGui::GetContentRegionAvail().x);

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        float btnWidth = lineHeight + 3.0f;
        float xW = std::round(width * 0.5f) - btnWidth;

        ImVec2 buttonSize = { btnWidth, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            values.x = resetValue;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset");
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(xW);
        bool status = ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            values.y = resetValue;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reset");
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(width - xW - btnWidth * 2);
        status = ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawFloatControl(const std::string& label, float* values, float speed, float vmin, float vmax, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::DragFloat("##X", values, speed, vmin, vmax, "%.2f");
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawIntControl(const std::string& label, int* values, int vmin, int vmax, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::DragInt("##X", values, 1, vmin, vmax);
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawColor4Control(const std::string& label, glm::vec4& color, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::ColorEdit4("##colorPicker", glm::value_ptr(color),
            ImGuiColorEditFlags_AlphaBar)) {
            changed = true;
        }
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawAssetDragDropBox(const std::string& label, const std::string& text, AssetHandle* handle, AssetType requireType, float columnWidth)
    {
        return DrawAssetDragDropBox(label, text, handle, { requireType }, nullptr, columnWidth);
    }

    bool DrawAssetDragDropBox(const std::string& label, const std::string& text, AssetHandle* handle, std::vector<AssetType> requireType, AssetType* outType, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;

        ImVec2 xLabelSize = ImGui::CalcTextSize("X");
        float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;

        bool isValided = AssetManager::IsAssetHandleValid(*handle);

        if (ImGui::Button(text.c_str(), { ImGui::GetContentRegionAvail().x - (isValided ? buttonSize : 0), lineHeight })) {
            //FileSystemWindow::localPath = path1.parent_path().string();
            // TODO 资源面板打开对应路径
        }

        if (ImGui::IsItemHovered() && !text.empty()) {
            ImGui::SetTooltip("%s", text.c_str());
        }

        if (ImGui::BeginDragDropTarget())
        {
            // TODO 添加类别判断
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
            {
                AssetHandle handle_ = *(AssetHandle*)payload->Data;

                for (auto& type : requireType)
                {
                    if (AssetManager::GetAssetType(handle_) == type)
                    {
                        if (*handle != handle_)
                        {
                            changed = true;
                            if (outType) *outType = type;
                            *handle = handle_;
                        }
                    }
                    else
                    {
                        NS_CORE_WARN("错误的资源类型！");
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (isValided)
        {
            ImGui::SameLine();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
            if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
            {
                changed = true;
                if (outType) *outType = AssetType::None;
                *handle = 0;
            }
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Clear");
        }

        NS_IMGUI_FIELD_END;
        return changed;
    }

    bool DrawInputText(const std::string& label, char* buf, size_t buf_size, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::InputText("##inputText", buf, buf_size);
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawInputTextMultiline(const std::string& label, std::string* buf, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::InputTextMultiline("##inputText", buf);
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawCheckbox(const std::string& label, bool* value, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        changed = ImGui::Checkbox("##checkbox", value);

        NS_IMGUI_FIELD_END;

        return changed;
    }

    bool DrawCombo(const std::string& label, const char** allValues, uint32_t* currentIndex, uint32_t valueNum, float columnWidth)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::BeginCombo("##combo", allValues[*currentIndex]))
        {
            for (int i = 0; i < valueNum; i++)
            {
                bool isSelected = *currentIndex == i;
                if (ImGui::Selectable(allValues[i], isSelected))
                {
                    *currentIndex = i;
                    changed = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }
}