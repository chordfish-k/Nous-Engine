# [Nous]
project(Nous)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")

# 添加子模块
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/yaml-cpp)

option(BOX2D_BUILD_UNIT_TESTS OFF)
option(BOX2D_BUILD_TESTBED OFF)
add_subdirectory(vendor/box2d)


# 源码搜集
file(GLOB_RECURSE SOURCES
        src/*.h
        src/*.cpp
        vendor/stb_image/*.h
        vendor/stb_image/*.cpp
        vendor/glm/glm/*.hpp
        vendor/glm/glm/*.inl

        vendor/imgui/imconfig.h
        vendor/imgui/imgui.h
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_internal.h
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imstb_rectpack.h
        vendor/imgui/imstb_textedit.h
        vendor/imgui/imstb_truetype.h
        vendor/imgui/imgui_demo.cpp

        vendor/imGuizmo/ImGuizmo.h
        vendor/imGuizmo/ImGuizmo.cpp
)

if (WIN32)
    file(GLOB_RECURSE SOURCES
            ${SOURCES}
            src/Platform/Windows/*.cpp
    )
endif ()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}"
)

# c++等级
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# 宏定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NS_DEBUG_LOG_FILE
        GLFW_INCLUDE_NONE
)
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE

    )
endif ()


# 预编译头
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/pch.h")


target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# 添加依赖包含目录
target_include_directories(${PROJECT_NAME} PUBLIC
        src
        vendor/spdlog/include
        vendor/GLFW/include
        vendor/glad/include
        vendor/imgui
        vendor/glm
        vendor/stb_image
        vendor/entt
        vendor/yaml-cpp/include
        vendor/imGuizmo
        vendor/box2d/include
)

# 获取Vulkan环境变量
set(VULKAN_SDK "$ENV{VULKAN_SDK}")
# 添加Vulkan依赖
if (DEFINED ENV{VULKAN_SDK})
    message("VulkanSDK:${VULKAN_SDK}")
    target_include_directories(${PROJECT_NAME} PUBLIC
            ${VULKAN_SDK}/Include
    )
else ()
    message("没有安装VulkanSDK")
endif ()

# 链接依赖库
set(VULKAN_LIB "${VULKAN_SDK}/Lib")
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glad
#        ImGui
        yaml-cpp
        box2d
        opengl32.lib
        ${VULKAN_LIB}/vulkan-1.lib
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME} PRIVATE
            # vulkan
            ${VULKAN_LIB}/spirv-cross-cored.lib
            ${VULKAN_LIB}/spirv-cross-glsld.lib
            ${VULKAN_LIB}/shaderc_sharedd.lib
    )
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(${PROJECT_NAME} PRIVATE
            # vulkan
            ${VULKAN_LIB}/spirv-cross-core.lib
            ${VULKAN_LIB}/spirv-cross-glsl.lib
            ${VULKAN_LIB}/shaderc_shared.lib
    )
endif ()

# 获取所有imGuizmo匹配的文件
file(GLOB_RECURSE IMGUIZMO_SOURCES "vendor/imGuizmo/*.cpp")
# 为匹配的文件禁用预编译头
foreach(SOURCE_FILE ${IMGUIZMO_SOURCES})
    set_source_files_properties(${SOURCE_FILE} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endforeach()

# 针对不同的构建类型设置编译选项
if (MSVC)
    # 静态运行时库
    foreach(flag_var
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
        if(${flag_var} MATCHES "/MDd")
            string(REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_DEBUG)
    if(MSVC)
        string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
    endif ()
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_RELEASE)
    if(MSVC)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        target_compile_options(${PROJECT_NAME} PRIVATE "/Ox")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE "-O3")
    endif()
endif()