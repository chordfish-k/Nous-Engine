#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace Nous::UI
{
	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, color);
		}

		ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, color);
		}

		~ScopedStyleColor()
		{
			if (m_Set)
				ImGui::PopStyleColor();
		}
	private:
		bool m_Set = false;
	};

#define NS_IMGUI_FIELD_BEGIN                    \
    ImGui::PushID(label.c_str());               \
    ImGui::Columns(2);                          \
    ImGui::SetColumnWidth(0, columnWidth);      \
    ImGui::Text("%s", label.c_str());           \
    if (ImGui::IsItemHovered())                 \
        ImGui::SetTooltip("%s", label.c_str()); \
    ImGui::NextColumn();                        \
    ImGui::PushStyleVar                         \
    (ImGuiStyleVar_ItemSpacing, ImVec2{ 0, GImGui->Style.FramePadding.y })

#define NS_IMGUI_FIELD_END                      \
    ImGui::PopStyleVar();                       \
    ImGui::Columns(1);                          \
    ImGui::PopID()

    static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
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

    static bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f)
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
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::PushItemWidth(width - xW - btnWidth*2);
        status = ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        changed = changed || status;
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawFloatControl(const std::string& label, float* values, float speed = 0.1f, float vmin = 0.0f, float vmax = 0.0f, float columnWidth = 100.0f)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::DragFloat("##X", values, speed, vmin, vmax, "%.2f");
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawColor4Control(const std::string& label, glm::vec4& color, float columnWidth = 100.0f)
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

    static bool DrawResourceDragDropBox(const std::string& label, const std::string& type, std::string& path, float columnWidth = 100.0f)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;

        std::filesystem::path path1 = path;
        std::string name = path1.filename().string();
        if (ImGui::Button(name.c_str(), { ImGui::GetContentRegionAvail().x, lineHeight })) {
            //FileSystemWindow::localPath = path1.parent_path().string();
        }

        if (ImGui::IsItemHovered() && !name.empty()) {
            ImGui::SetTooltip("%s", name.c_str());
        }

        if (ImGui::BeginDragDropTarget())
        {
            // TODO Ìí¼ÓÀà±ðÅÐ¶Ï
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.c_str()))
            {
                const wchar_t* path_ = (const wchar_t*)payload->Data;
                std::filesystem::path filepath(path_);
                path = filepath.string();
                changed = true;
            }
            ImGui::EndDragDropTarget();
        }
        //ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawInputText(const std::string& label, char* buf, size_t buf_size, float columnWidth = 100.0f)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::InputText("##inputText", buf, buf_size);
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawInputTextMultiline(const std::string& label, std::string* buf, float columnWidth = 100.0f)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        changed = ImGui::InputTextMultiline("##inputText", buf);
        ImGui::PopItemWidth();

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawCheckbox(const std::string& label, bool* value, float columnWidth = 100.0f)
    {
        bool changed = false;

        NS_IMGUI_FIELD_BEGIN;

        changed = ImGui::Checkbox("##checkbox", value);

        NS_IMGUI_FIELD_END;

        return changed;
    }

    static bool DrawCombo(const std::string& label, const char** allValues, uint32_t* currentIndex, uint32_t valueNum, float columnWidth = 100.0f)
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