
-- Nous Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/Nous/vendor/stb_image"
IncludeDir["yaml_cpp"] = "%{wks.location}/Nous/vendor/yaml-cpp/include"
IncludeDir["Box2D"] = "%{wks.location}/Nous/vendor/box2d/include"
IncludeDir["filewatch"] = "%{wks.location}/Nous/vendor/filewatch"
IncludeDir["GLFW"] = "%{wks.location}/Nous/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Nous/vendor/glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Nous/vendor/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Nous/vendor/imguizmo"
IncludeDir["glm"] = "%{wks.location}/Nous/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/Nous/vendor/entt"
IncludeDir["mono"] = "%{wks.location}/Nous/vendor/mono/include"
IncludeDir["shaderc"] = "%{wks.location}/Nous/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Nous/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["msdfgen"] = "%{wks.location}/Nous/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["artery_font_format"] = "%{wks.location}/Nous/vendor/msdf-atlas-gen/artery-font-format"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Nous/vendor/msdf-atlas-gen/msdf-atlas-gen"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["mono"] = "%{wks.location}/Nous/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"