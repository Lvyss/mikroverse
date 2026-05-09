cmake_minimum_required(VERSION 3.16)
project(Microverse LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Output binary to project root ──────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ── Source files ───────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/glad.c
)

add_executable(Microverse ${SOURCES})

# ── Include paths ──────────────────────────────────────────────────────────
target_include_directories(Microverse PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# ── Libraries ──────────────────────────────────────────────────────────────
# GLFW (static)
target_link_libraries(Microverse PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
)

# Assimp (dynamic import lib)
target_link_libraries(Microverse PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/libassimp.dll.a
)

# OpenGL + system libs (MinGW / Windows)
if (WIN32)
    target_link_libraries(Microverse PRIVATE
        opengl32
        gdi32
        user32
        shell32
        # winmm is needed by GLFW on some configs
    )
endif()

# ── Compiler flags ─────────────────────────────────────────────────────────
if (MSVC)
    target_compile_options(Microverse PRIVATE /W3)
else()
    target_compile_options(Microverse PRIVATE -Wall -Wno-unused-variable)
endif()

# ── Copy DLL next to exe after build (Windows)B ─────────────────────────────
if (WIN32)
    add_custom_command(TARGET Microverse POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/libassimp-6.dll"
            "$<TARGET_FILE_DIR:Microverse>"
        COMMENT "Copying libassimp-6.dll to output directory"
    )
endif()

message(STATUS "=== Microverse Build Config ===")
message(STATUS "  Source dir : ${CMAKE_SOURCE_DIR}")
message(STATUS "  Output dir : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  C++ std    : ${CMAKE_CXX_STANDARD}")