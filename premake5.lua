include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Nous"
	architecture "x86_64"
	startproject "Nous-Editor"

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

group "Dependencies"
	include "vendor/premake"
	include "Nous/vendor/box2d"
	include "Nous/vendor/GLFW"
	include "Nous/vendor/glad"
	include "Nous/vendor/imgui"
	include "Nous/vendor/yaml-cpp"
group ""

group "Core"
	include "Nous"
	include "Nous-ScriptCore"
group ""

group "Tools"
	include "Nous-Editor"
group ""

group "Misc"
	include "SandBox"
group ""