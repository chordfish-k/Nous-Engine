cmake_minimum_required(VERSION 3.28)
project(NousEngine)

# 获取操作系统名称
if (WIN32)
    set(SYSTEM_NAME "Windows")
elseif (APPLE)
    set(SYSTEM_NAME "MacOS")
elseif (UNIX)
    set(SYSTEM_NAME "Linux")
endif()

# 获取体系结构
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
else ()
    set(ARCHITECTURE "x86")
endif()

# 设置输出目录
set(OUTPUT_DIR "${CMAKE_BUILD_TYPE}-${SYSTEM_NAME}-${ARCHITECTURE}")

# 添加子模块
set(VENDOR_DIR Nous/vendor)

set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_SHARED_LIBRARY ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
add_subdirectory(${VENDOR_DIR}/GLFW)
add_subdirectory(${VENDOR_DIR}/glad)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# [Nous]
set(MODULE_NAME Nous)
add_library(${MODULE_NAME} SHARED)
set_target_properties(${MODULE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${MODULE_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${MODULE_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${MODULE_NAME}"
)

# 宏定义
if (WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE
            NS_PLATFORM_WINDOWS
            NS_BUILD_DLL
            GLFW_INCLUDE_NONE
    )
endif ()


# 预编译头
target_precompile_headers(${MODULE_NAME} PRIVATE "${MODULE_NAME}/src/pch.h")

# 源码搜集
file(GLOB_RECURSE SOURCES "${MODULE_NAME}/src/**")
target_sources(${MODULE_NAME} PRIVATE ${SOURCES})

# 添加依赖包含目录
target_include_directories(${MODULE_NAME} PUBLIC
        ${MODULE_NAME}/src
        ${MODULE_NAME}/vendor/spdlog/include
        ${MODULE_NAME}/vendor/GLFW/include
        ${MODULE_NAME}/vendor/glad/include
)

# 链接依赖库
target_link_libraries(${MODULE_NAME} PRIVATE
        glfw
        glad
        opengl32.lib
)



# [SandBox]
set(MODULE_NAME SandBox)
add_executable(${MODULE_NAME})
set_target_properties(${MODULE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${MODULE_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${MODULE_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${MODULE_NAME}"
)

# 宏定义
if (WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE
            NS_PLATFORM_WINDOWS
    )
endif ()

# 连接库
target_link_libraries(${MODULE_NAME} PRIVATE
        Nous
)

# 添加依赖的头文件
target_include_directories(${MODULE_NAME} PRIVATE
        Nous/src
        Nous/vendor/spdlog/include
)

# 源码搜集
file(GLOB_RECURSE SOURCES "SandBox/src/**")
target_sources(${MODULE_NAME} PRIVATE ${SOURCES})

# 针对MSVC，确保DLL导出所有符号
if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    foreach(TARGET_NAME ${MODULE_NAME})
        set_target_properties(${MODULE_NAME} PROPERTIES
                WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endforeach()
endif()

# 添加构建后命令：将所有DLL复制到SandBox的二进制文件目录
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Nous>"
        "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/SandBox/$<TARGET_FILE_NAME:Nous>"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:glfw>"
        "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/SandBox/$<TARGET_FILE_NAME:glfw>"
)