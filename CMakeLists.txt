#cmake_minimum_required(VERSION 3.15)
#
#project(OpenGLPlayground
#    VERSION 0.1.0
#    LANGUAGES C CXX
#)
#
## --------------------
## Global settings
## --------------------
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#cmake_policy(SET CMP0072 NEW)
#set(OpenGL_GL_PREFERENCE "GLVND")
#
## --------------------
## Executable
## --------------------
#add_executable(OpenGLPlayground
#    src/main.cpp
#    src/engine/engine.cpp
#    src/window/window.cpp
#    src/rendering/renderer.cpp
#
#    src/camera/camera.cpp
#    src/input-manager/input-manager.cpp
#
#    src/models/asset-manager.cpp
#    src/models/mesh.cpp
#    src/models/model.cpp
#    src/models/scene.cpp
#
#    src/networking/networking.cpp
#    src/networking/client-logic.cpp
#    src/networking/server-logic.cpp
#
#    include/glad/glad.c
#    include/stb/stb_image.cpp
#)
#
## --------------------
## Includes
## --------------------
#target_include_directories(OpenGLPlayground PRIVATE
#    ${PROJECT_SOURCE_DIR}/include
#    ${PROJECT_SOURCE_DIR}/include/stb
#)
#
## --------------------
## Dependencies (via vcpkg)
## --------------------
#find_package(OpenGL REQUIRED)
#find_package(glfw3 CONFIG REQUIRED)
#find_package(assimp CONFIG REQUIRED)
#find_package(OpenSSL REQUIRED)
#find_package(msgpack-cxx CONFIG REQUIRED)
#find_package(Boost CONFIG REQUIRED)
#find_package(GameNetworkingSockets CONFIG REQUIRED)
#
#target_link_libraries(OpenGLPlayground PRIVATE
#    OpenGL::GL
#    glfw
#    assimp
#    OpenSSL::SSL
#    OpenSSL::Crypto
#    msgpack-cxx
#    Boost::boost
#    GameNetworkingSockets::GameNetworkingSockets
#)
#
## --------------------
## Platform defines
## --------------------
#if (WIN32)
#    target_compile_definitions(OpenGLPlayground PRIVATE PLATFORM_WINDOWS)
#elseif (UNIX)
#    target_compile_definitions(OpenGLPlayground PRIVATE PLATFORM_LINUX)
#endif()
#
## --------------------
## Shaders
## --------------------
#file(GLOB SHADERS
#    "${PROJECT_SOURCE_DIR}/src/shaders/*.vert"
#    "${PROJECT_SOURCE_DIR}/src/shaders/*.frag"
#)
#
#foreach(SHADER_FILE ${SHADERS})
#    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
#    configure_file(
#        ${SHADER_FILE}
#        ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}
#        COPYONLY
#    )
#endforeach()
#
#target_compile_definitions(OpenGLPlayground PRIVATE
#    SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
#    ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
#)
#


cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(OpenGLPlayground)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(include/GameNetworkingSockets)

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
    include/glad/glad.c
    include/stb/stb_image.cpp
    )

# Include paths
include_directories(include)
include_directories(include/stb)

include_directories(include/GameNetworkingSockets/include)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(msgpack-cxx CONFIG REQUIRED)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Link GLFW and OpenGL
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(OpenGLPlayground PRIVATE glfw OpenGL::GL assimp GameNetworkingSockets OpenSSL::Crypto OpenSSL::SSL msgpack-cxx )
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
