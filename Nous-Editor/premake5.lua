project "Nous-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/Nous/vendor/spdlog/include",
		"%{wks.location}/Nous/src",
		"%{wks.location}/Nous/vendor",
		"%{IncludeDir.entt}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}"
	}

	links
	{
		"Nous"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "NS_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "NS_RELEASE"
		kind "WindowedApp"
		entrypoint "mainCRTStartup"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "NS_DIST"
		runtime "Release"
		optimize "on"