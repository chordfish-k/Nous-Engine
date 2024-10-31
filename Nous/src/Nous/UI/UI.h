#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

#include "Nous/Asset/Asset.h"
#include "Nous/Asset/AssetManager.h"

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

	bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

    bool DrawVec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f, float columnWidth = 100.0f);

    bool DrawFloatControl(const std::string& label, float* values, float speed = 0.1f, float vmin = 0.0f, float vmax = 0.0f, float columnWidth = 100.0f);

    bool DrawColor4Control(const std::string& label, glm::vec4& color, float columnWidth = 100.0f);

    bool DrawAssetDragDropBox(const std::string& label, const std::string& text, AssetHandle* handle, AssetType requireType, float columnWidth = 100.0f);

    bool DrawInputText(const std::string& label, char* buf, size_t buf_size, float columnWidth = 100.0f);

    bool DrawInputTextMultiline(const std::string& label, std::string* buf, float columnWidth = 100.0f);

    bool DrawCheckbox(const std::string& label, bool* value, float columnWidth = 100.0f);

    bool DrawCombo(const std::string& label, const char** allValues, uint32_t* currentIndex, uint32_t valueNum, float columnWidth = 100.0f);
}