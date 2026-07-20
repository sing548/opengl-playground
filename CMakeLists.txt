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
    src/engine/audio/audio-manager.cpp
    src/engine/audio/miniaudio-impl.cpp
    src/engine/camera/camera.cpp
    src/engine/window/window.cpp
    src/engine/helpers/file-helper.cpp
    src/engine/input-manager/input-manager.cpp
    src/engine/models/asset-manager.cpp
    src/engine/models/mesh.cpp
    src/engine/models/model.cpp
    src/engine/models/scene.cpp
    src/engine/net-transport/client-transport.cpp
    src/engine/net-transport/server-transport.cpp
    src/engine/rendering/sky/sky.cpp
    src/engine/rendering/renderer.cpp
    src/engine/rendering/terrain/chunk-handler.cpp
    src/engine/engine.cpp
    src/game/game-world/game-world.cpp
    src/game/networking/interpolator/interpolator.cpp
    src/game/networking/network-bridge/network-bridge.cpp
    src/game/rendering/flight/con-trail.cpp
    src/game/rendering/flight/drive-plume.cpp
    src/game/rendering/grass/grass.cpp
    src/game/rendering/materials/hitbox-material.cpp
    src/game/rendering/materials/model-material.cpp
    src/game/rendering/materials/terrain-material.cpp
    src/game/rendering/terrain/fbm-noise.cpp
    src/game/rendering/terrain/flat-chunk-generator.cpp
    src/game/rendering/terrain/terrain-handler.cpp
    src/game/rendering/vfx/death-explosion.cpp
    src/game/spawner/spawner.cpp
    src/game/systems/networking/network-input-consume-system.cpp
    src/game/systems/networking/network-input-distribution-system.cpp
    src/game/systems/networking/network-merge-system.cpp
    src/game/systems/networking/network-poll-system.cpp
    src/game/systems/networking/network-state-distribution-system.cpp
    src/game/systems/blending-system.cpp
    src/game/systems/camera-system.cpp
    src/game/systems/npc-system.cpp
    src/game/systems/physics-system.cpp
    src/game/systems/pi-history-system.cpp
    src/game/systems/player-system.cpp
    src/game/systems/shot-system.cpp
    src/game/systems/homing-system.cpp
    src/game/systems/terrain-system.cpp

    src/main.cpp
    include/glad/glad.c
    include/stb/stb_image.cpp

    include/ImGui/imgui.cpp
    include/ImGui/imgui_draw.cpp
    include/ImGui/imgui_tables.cpp
    include/ImGui/imgui_widgets.cpp
    include/ImGui/imgui_demo.cpp
    include/ImGui/backends/imgui_impl_glfw.cpp
    include/ImGui/backends/imgui_impl_opengl3.cpp
)

# Modern target-based includes
target_include_directories(OpenGLPlayground PRIVATE
    include
    include/stb
    include/miniaudio
    include/GameNetworkingSockets/include
    include/ImGui
    include/ImGui/backends
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

# miniaudio's runtime backends need these on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(OpenGLPlayground PRIVATE dl pthread m)
endif()

# Make the binary relocatable on Linux: search for bundled .so files (e.g. the
# GameNetworkingSockets shared lib, or libs bundled into a libs/ subfolder) next
# to the executable itself, via $ORIGIN. CMake still adds the automatic build-tree
# rpath, so running from build/ keeps working; the extra $ORIGIN entries only kick
# in once the binary is copied elsewhere with its .so files alongside.
# ($ORIGIN is the Linux/ELF form; macOS would use @loader_path, Windows ignores rpath.)
if(UNIX AND NOT APPLE)
    set_target_properties(OpenGLPlayground PROPERTIES
        BUILD_RPATH   "$ORIGIN:$ORIGIN/libs"
        INSTALL_RPATH "$ORIGIN:$ORIGIN/libs")
endif()

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

# Sanitizers in explicitly created Debug builds,
# otherwise performance degredation is too big for debugging
# (Linux/macOS; GCC and Clang only)
option(ENABLE_SANITIZERS "Enable ASan + UBSan in Debug builds" OFF)

if(ENABLE_SANITIZERS AND NOT MSVC
   AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(OpenGLPlayground PRIVATE -Wall -Wextra
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

# Copy assets next to the executable at build time (runs only when the exe
# relinks). $<TARGET_FILE_DIR:...> resolves to the actual exe directory for any
# config (Debug/Release) and any platform, so this works everywhere.
add_custom_command(TARGET OpenGLPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:OpenGLPlayground>/assets"
    COMMENT "Copying assets to output directory"
)

# Shaders change constantly during development, so copy them on EVERY build:
# custom targets are always considered out-of-date, so this command re-runs even
# when no source changed and the exe wasn't relinked. copy_directory is instant
# for small text files. (Assets stay on POST_BUILD above — re-copying multi-MB
# skyboxes every build would be wasted I/O, and they rarely change.)
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/shaders"
        "$<TARGET_FILE_DIR:OpenGLPlayground>/shaders"
    COMMENT "Copying shaders to output directory"
)
# Build the exe first so its output directory exists before we copy into it.
add_dependencies(copy_shaders OpenGLPlayground)
