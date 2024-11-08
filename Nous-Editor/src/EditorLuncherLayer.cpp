#include "EditorLuncherLayer.h"

#include "Nous/Core/Application.h"
#include "Nous/UI/UI.h"
#include "Nous/Utils/PlatformUtils.h"

#include "Panel/DockingSpace.h"

namespace Nous
{
	EditorLuncherLayer::EditorLuncherLayer()
		:m_Spec({}), Layer(nullptr,"EditorLuncherLayer")
	{
	}

	EditorLuncherLayer::EditorLuncherLayer(Application* application, const ApplicationSpecification& spec)
		: m_Spec(spec), Layer(application, "EditorLuncherLayer")
	{
	}

	void EditorLuncherLayer::OnAttached()
	{
	}

	void EditorLuncherLayer::OnDetached()
	{
	}

	void EditorLuncherLayer::OnUpdate(Timestep dt)
	{
	}

	void EditorLuncherLayer::OnImGuiRender()
	{
		NS_PROFILE_FUNCTION();

		DockingSpace::BeginDocking(false);

		ImGui::Begin("Luncher", 0, ImGuiWindowFlags_NoScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

		auto winSize = ImGui::GetContentRegionAvail();
		ImGui::Button("New Project", { winSize.x, winSize.y / 2 });
		if (ImGui::Button("Load Project", { winSize.x, winSize.y / 2 }))
		{
			std::filesystem::path editorPath = m_Spec.CommandLineArgs[0];
			std::filesystem::path path = FileDialogs::OpenFile("Nous Project(*.nsproj)\0 * .nsproj\0");
#ifdef NS_PLATFORM_WINDOWS
			std::string cmd = fmt::format("start \"\" \"{0}\" {1}", editorPath, path);
			std::system(cmd.c_str());
#endif
			m_Application->Close();
		}

		ImGui::PopStyleVar(2);
		ImGui::End();

		DockingSpace::EndDocking();
	}

	void EditorLuncherLayer::OnEvent(Event& event)
	{
	}
}