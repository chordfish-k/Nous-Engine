include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

<<<<<<< HEAD
workspace "Nous"
	architecture "x86_64"
	startproject "Nous-Editor"
=======
workspace "Hazel"
	architecture "x86_64"
	startproject "Hazelnut"
>>>>>>> premake

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
<<<<<<< HEAD
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
=======
	include "Hazel/vendor/Box2D"
	include "Hazel/vendor/GLFW"
	include "Hazel/vendor/Glad"
	include "Hazel/vendor/msdf-atlas-gen"
	include "Hazel/vendor/imgui"
	include "Hazel/vendor/yaml-cpp"
group ""

group "Core"
	include "Hazel"
	include "Hazel-ScriptCore"
group ""

group "Tools"
	include "Hazelnut"
group ""

group "Misc"
	include "Sandbox"
>>>>>>> premake
group ""