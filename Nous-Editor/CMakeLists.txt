# [Nous-Editor]
project(Nous-Editor)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${OUTPUT_DIR}/${PROJECT_NAME}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME}")

# 源码搜集
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

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
    )
endif ()

# 连接库
target_link_libraries(${PROJECT_NAME} PRIVATE
        Nous
)

# 添加依赖的头文件
target_include_directories(${PROJECT_NAME} PRIVATE
        ../Nous/src
        ../Nous/vendor/spdlog/include
        ../Nous/vendor
        ../Nous/vendor/glm
        ../Nous/vendor/entt
)


# 针对MSVC，确保DLL导出所有符号
if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    foreach(TARGET_NAME ${PROJECT_NAME})
        set_target_properties(${PROJECT_NAME} PROPERTIES
                WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endforeach()
endif()

# 针对不同的构建类型设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 添加宏定义
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_DEBUG)

    # 设置为使用Debug运行时库
    # 注意：在CMake中，通常不需要显式设置使用Debug或Release运行时库
    # 但如果你确实需要，可以通过以下方式设置（针对MSVC）：
    if(MSVC)
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
    endif ()
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 添加宏定义
    target_compile_definitions(${PROJECT_NAME} PRIVATE NS_RELEASE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-O3")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE "/Ox")
    endif()
endif()