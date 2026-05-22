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

# Must be set BEFORE add_subdirectory so the GNS DLL lands in the same
# directory as the executable (Windows DLL search starts in the exe's dir).
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

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
    src/engine/net-transport/cli-transp.cpp
    src/engine/net-transport/serv-transp.cpp
    src/engine/networking/client-transport.cpp
    src/engine/networking/server-transport.cpp
    src/engine/rendering/materials/hitbox-material.cpp
    src/engine/rendering/materials/model-material.cpp
    src/engine/rendering/sky/sky.cpp
    src/engine/rendering/renderer.cpp
    src/engine/engine.cpp
    src/game/game-world/game-world.cpp
    src/game/networking/network-bridge/netw-bridg.cpp
    src/game/networking/network-bridge/network-bridge.cpp
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

# Windows: copy all vcpkg imported DLLs next to the executable after each build.
# GNS DLL is already there because CMAKE_RUNTIME_OUTPUT_DIRECTORY is set above.
# $<TARGET_RUNTIME_DLLS:...> only covers imported targets (vcpkg), not in-tree ones.
if(WIN32)
    add_custom_command(TARGET OpenGLPlayground POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:OpenGLPlayground>
            $<TARGET_FILE_DIR:OpenGLPlayground>
        COMMAND_EXPAND_LISTS
    )
endif()

# Sanitizers in Debug builds (Linux/macOS; GCC and Clang only)
option(ENABLE_SANITIZERS "Enable ASan + UBSan in Debug builds" ON)

if(ENABLE_SANITIZERS AND NOT MSVC
   AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(OpenGLPlayground PRIVATE
        -fsanitize=address,undefined
        -fno-sanitize=vptr  
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all      # abort on first error, don't just print
    )
    target_link_options(OpenGLPlayground PRIVATE
        -fsanitize=address,undefined
        -fno-sanitize=vptr  
    )
endif()

# Copy shaders and assets next to the executable at build time.
# $<TARGET_FILE_DIR:...> resolves to the actual exe directory for any
# config (Debug/Release) and any platform, so this works everywhere.
add_custom_command(TARGET OpenGLPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/shaders"
        "$<TARGET_FILE_DIR:OpenGLPlayground>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:OpenGLPlayground>/assets"
    COMMENT "Copying shaders and assets to output directory"
)
