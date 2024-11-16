#include "SceneManagerDialog.h"

#include "Nous/Project/Project.h"
#include "Nous/Asset/EditorAssetManager.h"

#include <imgui/imgui.h>

namespace Nous
{
	static bool s_Open = false;

	void SceneManagerDialog::Open()
	{
		s_Open = true;
	}

	void SceneManagerDialog::OnImGuiRender()
	{
		if (!s_Open)
			return;

		ImGui::Begin("Scene Manager", &s_Open, ImGuiWindowFlags_NoDocking);
		
		ImGui::Text("Default Scene");

		AssetRegistry& reg = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		AssetHandle& startScene = Project::GetActive()->GetConfig().StartScene;

		for (auto& [handle, metadata] : reg)
		{
			if (metadata.Type != AssetType::Scene)
				continue;

			bool is = startScene == handle;
			if (ImGui::Checkbox(metadata.FilePath.filename().string().c_str(), &is))
			{
				startScene = handle;
				Project::SaveActive();
			}
		}

		ImGui::End();
	}
}