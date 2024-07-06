cmake_minimum_required(VERSION 3.28)
project(NousEngine)

# 宏定义
if (WIN32)
    add_definitions(-DNS_PLATFORM_WINDOWS -DNS_BUILD_DLL)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子模块
add_subdirectory(./Nous/vendor/spdlog)

# [Nous]
add_library(Nous SHARED)
set_target_properties(Nous PROPERTIES
        OUTPUT_NAME "Nous" # 设置输出的库文件名
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # 设置静态库输出目录
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # 设置动态库输出目录
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" # 设置可执行文件输出目录
)

# 预编译头
target_precompile_headers(Nous PRIVATE "Nous/src/pch.h")

# 源码搜集
file(GLOB_RECURSE SOURCES "Nous/src/**")
target_sources(Nous PRIVATE ${SOURCES})

#添加依赖包含目录
target_include_directories(Nous PUBLIC
        Nous/src
        Nous/vendor/spdlog/include
)

# 链接 spdlog 库
target_link_libraries(Nous PRIVATE spdlog)

# [SandBox]
add_executable(SandBox)
set_target_properties(SandBox PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # 设置静态库输出目录
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # 设置动态库输出目录
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# 源码搜集
file(GLOB_RECURSE HEADERS "SandBox/src/**")
target_sources(SandBox PRIVATE ${SOURCES} ${HEADERS})

# 连接库
target_link_libraries(SandBox PRIVATE Nous)

# 添加依赖的头文件
target_include_directories(SandBox PRIVATE
        Nous/src
        Nous/vendor/spdlog/include
)
