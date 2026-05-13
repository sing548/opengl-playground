cmake_minimum_required(VERSION 3.15) # Bumped for better vcpkg support
project(OpenGLPlayground LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json (VS Code loves this)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug build if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Fix for Linux GLVND, ignored by Windows
set(OpenGL_GL_PREFERENCE "GLVND")

# Find Dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(msgpack-cxx CONFIG REQUIRED)
# Note: msgpack-cxx's CMake config does a transitive find_dependency(Boost),
# so Boost is resolved automatically — we don't use Boost directly.

# GameNetworkingSockets (submodule). Build only the shared lib we actually link;
# tests/examples already default OFF but set explicitly for safety.
set(BUILD_STATIC_LIB OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIB ON  CACHE BOOL "" FORCE)
set(BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS      OFF CACHE BOOL "" FORCE)
add_subdirectory(include/GameNetworkingSockets)

# Define Executable
add_executable(OpenGLPlayground
    src/engine/camera/camera.cpp
    src/engine/window/window.cpp
    src/engine/helpers/file-helper.cpp
    src/engine/input-manager/input-manager.cpp
    src/engine/models/asset-manager.cpp
    src/engine/models/mesh.cpp
    src/engine/models/model.cpp
    src/engine/models/scene.cpp
    src/engine/networking/client-logic.cpp
    src/engine/networking/server-logic.cpp
    src/engine/networking/networking.cpp
    src/engine/rendering/materials/hitbox-material.cpp
    src/engine/rendering/materials/model-material.cpp
    src/engine/rendering/sky/sky.cpp
    src/engine/rendering/renderer.cpp
    src/engine/engine.cpp
    src/game/game-world/game-world.cpp
    src/game/spawner/spawner.cpp
    src/game/systems/camera-system.cpp
    src/game/systems/npc-system.cpp
    src/game/systems/physics-system.cpp
    src/game/systems/player-system.cpp
    src/game/systems/shot-system.cpp

    src/main.cpp
    include/glad/glad.c
    include/stb/stb_image.cpp
)

# Modern target-based includes
target_include_directories(OpenGLPlayground PRIVATE
    include
    include/stb
    include/GameNetworkingSockets/include
)

# Linking
# Note: glfw3 is the target name used by vcpkg
target_link_libraries(OpenGLPlayground PRIVATE
    glfw
    OpenGL::GL
    assimp::assimp
    OpenSSL::Crypto
    OpenSSL::SSL
    msgpack-cxx
    GameNetworkingSockets
)

# Windows specific: Copy DLLs to the output folder automatically
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Asset/Shader Handling
file(GLOB SHADERS "src/shaders/*.vert" "src/shaders/*.frag")

foreach(SHADER_FILE ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME} COPYONLY)
endforeach()

# 1. Find all files recursively in the assets folder
file(GLOB_RECURSE ASSET_FILES 
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "assets/*.mtl"
    "assets/*.obj"
    "assets/*.png"
    "assets/*.jpg"
)

# 2. Loop through and copy them while maintaining structure
foreach(ASSET_PATH ${ASSET_FILES})
    configure_file(
        "assets/${ASSET_PATH}" 
        "${CMAKE_CURRENT_BINARY_DIR}/assets/${ASSET_PATH}" 
        COPYONLY
    )
endforeach()

target_compile_definitions(OpenGLPlayground PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
    ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
