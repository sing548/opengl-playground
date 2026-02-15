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
find_package(Boost REQUIRED COMPONENTS system)

# GameNetworkingSockets (assuming submodule)
add_subdirectory(include/GameNetworkingSockets)

# Define Executable
add_executable(OpenGLPlayground
    src/camera/camera.cpp
    src/models/asset-manager.cpp
    src/models/mesh.cpp
    src/models/model.cpp
    src/models/scene.cpp
    src/rendering/renderer.cpp
    src/window/window.cpp
    src/main.cpp
    src/engine/engine.cpp
    src/input-manager/input-manager.cpp
    src/networking/networking.cpp
    src/networking/client-logic.cpp
    src/networking/server-logic.cpp
    src/helpers/file-helper.cpp
    include/glad/glad.c
    include/stb/stb_image.cpp
)

# Modern target-based includes
target_include_directories(OpenGLPlayground PRIVATE 
    include 
    include/stb
    include/GameNetworkingSockets/include
    ${Boost_INCLUDE_DIRS}
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
    ${Boost_LIBRARIES}
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

target_compile_definitions(OpenGLPlayground PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
    ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)
