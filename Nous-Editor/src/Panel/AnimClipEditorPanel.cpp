#include "pch.h"
#include "AnimClipEditorPanel.h"

#include "Nous/Utils/PlatformUtils.h"
#include "Nous/Renderer/Texture.h"
#include "Nous/Asset/AssetManager.h"
#include "Nous/UI/UI.h"

#include "Nous/Anim/AnimClipSerializer.h"

namespace Nous
{
	static bool s_Show = true;
	static Ref<AnimClip> s_Clip;

	void AnimClipEditorPanel::Open(AssetHandle clipHandle)
	{
		NS_CORE_ASSERT(AssetManager::GetAssetType(clipHandle) == AssetType::AnimClip);
		s_Show = true;
		s_Clip = AssetManager::GetAsset<AnimClip>(clipHandle);
		ImGui::SetWindowFocus("Anim Clip");
	}

	void AnimClipEditorPanel::OnImGuiRender()
	{
		if (!s_Show)
			return;

		ImGui::Begin("Anim Clip", &s_Show);

		// Save
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
		float padx = ImGui::GetStyle().FramePadding.x * 2;

		{
			float total = ImGui::GetContentRegionAvailWidth();
			float w = total / 4 - padx;
			if (ImGui::Button("New", { w, lineHeight }))
			{
				s_Clip = CreateRef<AnimClip>();
				s_Clip->Frames = std::vector<AnimFrame>();
				s_Clip->Handle = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload", { w, lineHeight }))
			{
				if (s_Clip && s_Clip->Handle)
				{
					Project::GetActive()->GetEditorAssetManager()->ReloadAsset(s_Clip->Handle);
					s_Clip = AssetManager::GetAsset<AnimClip>(s_Clip->Handle);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save", {w, lineHeight}))
			{
				if (!s_Clip->Handle)
				{
					NS_WARN("New File. Use Save As!");
				}
				else
				{
					AnimClipSerializer ser(s_Clip);
					ser.Serialize(Project::GetActiveAssetDirectory() / Project::GetActive()->GetEditorAssetManager()->GetFilePath(s_Clip->Handle));
				}
				
			}
			ImGui::SameLine();
			if (ImGui::Button("Save As", { total - w * 3 - padx, lineHeight}))
			{
				std::filesystem::path filepath = FileDialogs::SaveFile("Nous Anim Clip (*nsani)\0*.nsani\0");
				if (!filepath.empty())
				{
					AnimClipSerializer ser(s_Clip);
					ser.Serialize(filepath);

					/*filepath = std::filesystem::relative(filepath, Project::GetActiveAssetDirectory());
					auto handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(filepath);
					s_Clip = AssetManager::GetAsset<AnimClip>(handle);*/
				}
			}
		}
		
		if (!s_Clip)
		{
			ImGui::End();
			return;
		}
		ImGui::Separator();

		// Name
		static char nameBuffer[64];
		strcpy_s(nameBuffer, sizeof(nameBuffer), s_Clip->Name.c_str());
		if (UI::DrawInputText("Name", nameBuffer, sizeof(nameBuffer)))
			s_Clip->Name = nameBuffer;
		
		// Type
		const char* animClipTypeStrings[] = { "Single", "SpriteSheet" };
		uint32_t currentIndex = (int)s_Clip->Type;
		if (UI::DrawCombo("Type", animClipTypeStrings, &currentIndex, 2))
			s_Clip->Type = (AnimClipType)currentIndex;

		// Type:SpriteSheet
		if (s_Clip->Type == AnimClipType::SpriteSheet)
		{
			// Image
			AssetHandle handle = s_Clip->ImageHandle;
			if (UI::DrawAssetDragDropBox("Image", AssetManager::GetAssetFileName<Texture2D>(handle), &handle, AssetType::Texture2D))
				s_Clip->ImageHandle = handle;

			// Spec
			int width = s_Clip->SheetWidth, height = s_Clip->SheetHeight;
			if (UI::DrawIntControl("Sheet Width", &width) && width > 0)
				s_Clip->SheetWidth = width;
			if (UI::DrawIntControl("Sheet Height", &height) && height > 0)
				s_Clip->SheetHeight = height;
		}
		
		// Add Frame
		
		if (ImGui::Button("Add Frame", {-1, lineHeight}))
		{
			s_Clip->Frames.push_back({});
		}

		// Frames
		if (ImGui::TreeNodeEx("Frames", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i = 0;
			static int draging = -1;

			for (auto& frame : s_Clip->Frames)
			{
				ImGui::PushID(i);
					
				std::string idStr = fmt::format("{}: ", i);

				int index = frame.Index;
				float duration = frame.Duration;
				float labelW = ImGui::CalcTextSize("xxx:x").x;
				float totalW = ImGui::GetContentRegionAvailWidth();

				ImGui::PushItemWidth(labelW);
				ImGui::Text(idStr.c_str());
				ImGui::PopItemWidth();
				ImGui::SameLine();

				float dragBtnW = 50;
				float availW = totalW - labelW - dragBtnW;

				// Type:SpriteSheet
				if (s_Clip->Type == AnimClipType::SpriteSheet)
				{
					// Index
					ImGui::PushItemWidth(availW / 2 - padx);
					if (ImGui::DragInt("##Index", &index, 1, 0, 0, "idx: %d") && index > 0)
						frame.Index = index;
					ImGui::PopItemWidth();
				}
				else if (s_Clip->Type == AnimClipType::Single)
				{
					std::string name = AssetManager::GetAssetFileName<Texture2D>(frame.ImageHandle);
					ImGui::Button(name.c_str(), { availW / 2 - padx, lineHeight });
					if (ImGui::BeginDragDropTarget())
					{
						// TODO 添加类别判断
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_BROWSER_ITEM"))
						{
							AssetHandle handle_ = *(AssetHandle*)payload->Data;
							if (AssetManager::GetAssetType(handle_) == AssetType::Texture2D)
							{
								frame.ImageHandle = handle_;
							}
						}
						ImGui::EndDragDropTarget();
					}
				}

				// Duration
				ImGui::SameLine();
				ImGui::PushItemWidth(availW / 2 - padx);
				if (ImGui::DragFloat("##Duration", &duration, 0.01f, 0, 0, "dur: %.2f") && duration > 0.0f)
					frame.Duration = duration;
				ImGui::PopItemWidth();

				// Drag
				ImGui::SameLine();
				ImGui::Button("Drag", { ImGui::GetContentRegionAvailWidth(), lineHeight});
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("ANIM_CLIP_FRAME", &i, sizeof(i));
					ImGui::Text(fmt::format("frame {}", i).c_str());
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					draging = i;
					ImGui::EndDragDropSource();
				}

				bool shouldResetDraging = false;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth(), 10));
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
				if (draging >= 0 && ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					shouldResetDraging = true;
				}
					
				// 拖放处理
				ImGuiWindow* window = GImGui->CurrentWindow;
				auto originLastItemRect = window->DC.LastItemRect;
				window->DC.LastItemRect.Min.y += 7;
				window->DC.LastItemRect.Max.y -= 7;
				bool changed = false;
				bool open = ImGui::BeginDragDropTarget();
				window->DC.LastItemRect = originLastItemRect;

				if (open)
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIM_CLIP_FRAME"))
					{
						changed = true;
						int source = *(int*)payload->Data;
						int& target = i;
						auto& f = s_Clip->Frames;
						if (!(source == target || source == target + 1))
						{
							auto element = f[source];
							f.erase(f.begin() + source);

							// 重新计算插入位置
							// 如果 s 在 t 之前，那么 t 的位置在删除后会前移一位
							if (source < target) {
								target--;
							}

							// 插入到 t 元素后面
							f.insert(f.begin() + target + 1, element);
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
				ImGui::PopID();
				i++;

				if (changed)
					break;

				if (shouldResetDraging)
					draging = -1;
				
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}
}