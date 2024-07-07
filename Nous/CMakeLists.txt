# [Nous]
project(Nous)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")

# 添加子模块
set(VENDOR_DIR vendor)
set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_SHARED_LIBRARY ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
add_subdirectory(${VENDOR_DIR}/GLFW)
add_subdirectory(${VENDOR_DIR}/glad)
add_subdirectory(${VENDOR_DIR}/imgui)

# 源码搜集
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}"
)

# c++等级
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# 宏定义
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            NS_PLATFORM_WINDOWS
            NS_BUILD_DLL
            GLFW_INCLUDE_NONE
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
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glad
        ImGui
        opengl32.lib
)

# 针对不同的构建类型设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_DEBUG)
    if(MSVC)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
    endif ()
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_RELEASE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE "/Ox")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE "-O3")
    endif()
endif()