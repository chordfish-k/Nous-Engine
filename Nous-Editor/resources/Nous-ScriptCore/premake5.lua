project "Nous-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("../Nous-Editor/resources/scripts")
	objdir ("../Nous-Editor/resources/scripts/intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs",
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