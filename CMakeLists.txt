cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

project(OpenGLPlayground)

set(CMAKE_CXX_STANDARD 17)

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
    include/glad/glad.c
    include/stb/stb_image.cpp
    )

# Include paths
include_directories(include)
include_directories(include/stb)

# Link GLFW and OpenGL
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

target_link_libraries(OpenGLPlayground PRIVATE glfw OpenGL::GL assimp)
# Copy shader files to build directory
# Add shader files to be copied
file(GLOB SHADERS "src/shaders/*.vert" "src/shaders/*.frag")

# Copy shaders to build directory
foreach(SHADER_FILE ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME} COPYONLY)
endforeach()

# Define SHADER_DIR for your code
target_compile_definitions(OpenGLPlayground PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
    ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\"
)
