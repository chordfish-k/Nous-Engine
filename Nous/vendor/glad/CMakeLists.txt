project(glad)

set(GLAD_SOURCES
        include/glad/glad.h
        include/KHR/khrplatform.h
        src/glad.c
)

set(MODULE_NAME glad)
add_library(${MODULE_NAME} STATIC ${GLAD_SOURCES})

target_include_directories(${MODULE_NAME} PRIVATE
        include
)