cmake_minimum_required(VERSION 3.28)
project(NousEngine)

# 针对不同的构建类型设置编译选项
# 设置默认构建类型为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    add_compile_definitions(DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring for Release build")
    add_compile_definitions(NDEBUG)
endif()

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
set(OUTPUT_DIR "${SYSTEM_NAME}-${ARCHITECTURE}")

add_subdirectory(Nous)
add_subdirectory(SandBox)
