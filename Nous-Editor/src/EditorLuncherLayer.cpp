#include "EditorLuncherLayer.h"

#include "Nous/Core/Application.h"
#include "Nous/UI/UI.h"
#include "Nous/Utils/PlatformUtils.h"

#include "Panel/DockingSpace.h"

namespace Nous
{
	static std::filesystem::path s_EditorPath = std::filesystem::current_path();
	static std::string MakePremakeScript(const std::string& projName = "Sandbox") {
		return std::string() +
			"local RootDir = \"" + s_EditorPath.generic_string() + "\"\n" +
	"include(RootDir .. \"/resources/premake/premake_customization/solution_items.lua\")\n"
	"workspace \""+projName+"\"\n" +
	"architecture \"x86_64\"\n" +
	"startproject \""+projName+"\"" +
R"(
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	solution_items
	{
		".editorconfig"
	}
	flags
	{
		"MultiProcessorCompile"
	}
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
)" +
	"project \"" + projName + "\"\n" +
R"(
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Bin/App")
	objdir ("Bin/Temp")
	files
	{
		"./**.cs"
	}
	links
	{
		"Nous-ScriptCore"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "Off"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Off"
		symbols "Default"

group "Nous"
	include (RootDir .. "/resources/Nous-ScriptCore")
group ""
)";
	}

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
		
		if (ImGui::Button("New Project", { winSize.x, winSize.y / 2 }))
		{
			std::filesystem::path folder = FileDialogs::CreateFolder();
			if (std::filesystem::exists(folder))
			{
				auto proj = Project::New(folder);
				if (Project::SaveNew(proj))
				{
					// 写出premake5
					auto luaPath = folder / "premake5.lua";
					std::ofstream fout(luaPath);
					fout << MakePremakeScript(proj->GetConfig().Name);
					fout.close();

					// 生成sln
					auto c = fmt::format("cd {} && {}/resources/premake/premake/premake5.exe vs2022", folder, s_EditorPath.string());
					NS_CORE_INFO(c);
					std::system(c.c_str());
				
					// 删除premake5
					std::filesystem::remove(luaPath);

					// 启动编辑器本体
					std::filesystem::path path = folder / (proj->GetConfig().Name + ".nsproj");
					std::filesystem::path editorPath = m_Spec.CommandLineArgs[0];
	#ifdef NS_PLATFORM_WINDOWS
					std::string cmd = fmt::format("start \"\" \"{0}\" {1}", editorPath, path);
					std::system(cmd.c_str());
	#endif
				}
			}
			m_Application->Close();
		}

		if (ImGui::Button("Load Project", { winSize.x, winSize.y / 2 }))
		{
			std::filesystem::path editorPath = m_Spec.CommandLineArgs[0];
			std::filesystem::path path = FileDialogs::OpenFile("Nous Project(*.nsproj)\0 * .nsproj\0");
			if (std::filesystem::exists(path))
			{
#ifdef NS_PLATFORM_WINDOWS
			std::string cmd = fmt::format("start \"\" \"{0}\" {1}", editorPath, path);
			std::system(cmd.c_str());
#endif
			}
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