#include "pch.h"
#include "AnimMachineEditorPanel.h"

#include "Nous/Utils/PlatformUtils.h"
#include "Nous/Anim/AnimMachine.h"
#include "Nous/Anim/AnimMachineSerializer.h"
#include "Nous/ImGui/UI.h"

namespace Nous
{
	static bool s_Show = true;
	static Ref<AnimMachine> s_Machine;

	static void ShowConditionGui(Ref<ConditionNode> node)
	{
		// Type
		ConditionNodeType type = node->Type;

		// ConditionNodeType
		static const char* nodeTypeStrs[2] = { "Node", "Leaf" };
		uint32_t nodeTypeIndex = (int)node->Type - 1;

		if (EUI::DrawCombo("Node Type", nodeTypeStrs, &nodeTypeIndex, 2))
			node->Type = (ConditionNodeType)(nodeTypeIndex + 1);

		if (type == ConditionNodeType::Leaf)
		{
			// ValueType
			static const char* typeStrs[2] = { "Float", "Bool" };
			uint32_t typeIndex = (int)node->Exp.ValueType  - 1;
			if (typeIndex >= 2) typeIndex = 0;
			if (EUI::DrawCombo("Value Type", typeStrs, &typeIndex, 2))
				node->Exp.ValueType = (ValueType)(typeIndex + 1);

			// VarNam
			char buffer[32];
			strcpy_s(buffer, sizeof(buffer), node->Exp.VarName.c_str());
			if (EUI::DrawInputText("Var Name", buffer, sizeof(buffer)))
				node->Exp.VarName = buffer;

			if (node->Exp.ValueType == ValueType::Float)
			{
				// Compare
				static const char* symbolStrs[6] = {"==", "!=", "<", ">", ">=", "<="};
				uint32_t symbolIndex = (int)node->Exp.Compare - 1;
				if (EUI::DrawCombo("Compare", symbolStrs, &symbolIndex, 6))
					node->Exp.Compare = (FloatCompareType)(symbolIndex + 1);
			
				// TargetValue
				float value = node->Exp.TargetValue.FloatValue;
				if (EUI::DrawFloatControl("Value", &value))
					node->Exp.TargetValue.FloatValue = value;
			}
			else if(node->Exp.ValueType == ValueType::Bool)
			{
				bool value = node->Exp.TargetValue.BoolValue;
				if (EUI::DrawCheckbox("Is True", &value))
				{
					node->Exp.TargetValue.BoolValue = value;
					node->Exp.Compare = FloatCompareType::Equal;
				}
			}
		}
		else if (type == ConditionNodeType::Node)
		{
			// Symbol
			static const char* symbolStrs[2] = { "And", "Or" };
			uint32_t symbolIndex = (int)node->Symbol - 1;
			if (symbolIndex >= 2) symbolIndex = 0;
			if (EUI::DrawCombo("Symbol", symbolStrs, &symbolIndex, 2))
				node->Symbol = (BoolCompareType)(symbolIndex + 1);

			if (ImGui::TreeNode(fmt::format("Left##").c_str()))
			{
				if (!node->LeftChild)
					node->LeftChild = CreateRef<ConditionNode>();
				ShowConditionGui(node->LeftChild);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode(fmt::format("Right##").c_str()))
			{
				if (!node->RightChild)
					node->RightChild = CreateRef<ConditionNode>();
				ShowConditionGui(node->RightChild);
				ImGui::TreePop();
			}
		}
		
	}

	void AnimMachineEditorPanel::Open(AssetHandle handle)
	{
		NS_CORE_ASSERT(AssetManager::GetAssetType(handle) == AssetType::AnimMachine);
		s_Show = true;
		s_Machine = AssetManager::GetAsset<AnimMachine>(handle);
		ImGui::SetWindowFocus("Anim Machine");
	}

	void AnimMachineEditorPanel::OnImGuiRender()
	{
		if (!s_Show)
			return;

		ImGui::Begin("Anim Machine", &s_Show);
		
		// Save
		float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
		float padx = ImGui::GetStyle().FramePadding.x * 2;

		{
			float total = ImGui::GetContentRegionAvailWidth();
			float w = total / 4 - padx;
			if (ImGui::Button("New", { w, lineHeight }))
			{
				s_Machine = CreateRef<AnimMachine>();
				s_Machine->Handle = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload", { w, lineHeight }))
			{
				if (s_Machine && s_Machine->Handle)
				{
					Project::GetActive()->GetEditorAssetManager()->ReloadAsset(s_Machine->Handle);
					s_Machine = AssetManager::GetAsset<AnimMachine>(s_Machine->Handle);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save", { w, lineHeight }))
			{
				if (!s_Machine->Handle)
				{
					NS_WARN("New File. Use Save As!");
				}
				else
				{
					AnimMachineSerializer ser(s_Machine);
					ser.Serialize(Project::GetActiveAssetDirectory() / Project::GetActive()->GetEditorAssetManager()->GetFilePath(s_Machine->Handle));
				}

			}
			ImGui::SameLine();
			if (ImGui::Button("Save As", { total - w * 3 - padx, lineHeight }))
			{
				std::filesystem::path filepath = FileDialogs::SaveFile("Nous State Machine (*nsanm)\0*.nsanm\0");
				if (!filepath.empty())
				{
					AnimMachineSerializer ser(s_Machine);
					ser.Serialize(filepath);

					/*filepath = std::filesystem::relative(filepath, Project::GetActiveAssetDirectory());
					auto handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(filepath);
					s_Clip = AssetManager::GetAsset<AnimMachine>(handle);*/
				}
			}

			if (!s_Machine)
			{
				ImGui::End();
				return;
			}
			ImGui::Separator();
		}

		// Default State Index
		{
			int defaultIndex = s_Machine->DefaultIndex;
			if (EUI::DrawIntControl("Default Index", &defaultIndex) && defaultIndex >= 0 && defaultIndex < s_Machine->GetAllStates().size())
				s_Machine->DefaultIndex = defaultIndex;
		}

		// Add State
		if (ImGui::Button("Add State", { -1, lineHeight }))
		{
			s_Machine->GetAllStates().push_back({ 0, {} });
		}
		

		// States
		auto& states = s_Machine->GetAllStates();
		int i = 0;
		int deleteStateIdx = -1;

		for (auto& state : states)
		{
			if (ImGui::TreeNode(fmt::format("State {}", i).c_str()))
			{
				// Clip
				AssetHandle clipHandle = state.Clip;
				std::string label = "None";
				if (clipHandle)
				{
					if (AssetManager::IsAssetHandleValid(clipHandle))
						label = AssetManager::GetAsset<AnimClip>(clipHandle)->Name;
					else
						label = "Invalid";
				}
				if (EUI::DrawAssetDragDropBox("Clip", label, &clipHandle, AssetType::AnimClip))
					state.Clip = clipHandle;

				// Conditions
				auto& cons = state.Conditions;
				int j = 0;

				// Add Condition
				if (ImGui::Button("Add Condition"))
				{
					auto node = CreateRef<ConditionNode>();
					cons.push_back({ 0, node });
				}
				ImGui::SameLine();
				// Delete State
				if (ImGui::Button("Delete State"))
				{
					deleteStateIdx = i;
				}

				int deleteIdx = -1;
				for (auto& con : cons)
				{
					if (ImGui::TreeNode(fmt::format("Condition {0} ##{1}", j, i).c_str()))
					{
						int nextIndex = con.StateIndex;
						if (EUI::DrawIntControl("Next State", &nextIndex) && nextIndex >= 0 && nextIndex < s_Machine->GetAllStates().size())
							con.StateIndex = nextIndex;

						// Condition
						ShowConditionGui(con.ConditionRoot);

						if (ImGui::Button("Delete"))
						{
							deleteIdx = j;
						}
						ImGui::TreePop();
					}
					j++;
				}

				if (deleteIdx > -1)
				{
					cons.erase(cons.begin() + deleteIdx);
				}

				ImGui::TreePop();
			}
			i++;
		}

		if (deleteStateIdx >= 0)
		{
			states.erase(states.begin() + deleteStateIdx);
		}

		ImGui::End();
	}
}