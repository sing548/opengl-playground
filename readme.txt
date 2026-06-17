This is a small passion/learning project for graphics programming using C++ and OpenGL.
Currently mostly the architecture containing a basic pipeline handling rendering, shaders and gameplay systems and networking using Valve's GameNetworkingSockets.

What's implemented so far:
- Rendering pipeline with shader handling
- Model loading (assimp)
- Client/server networking (Valve's GameNetworkingSockets)
- Message serialization (protobuf / msgpack)
- Basic gameplay systems
- CMake build with optional vcpkg, Linux & Windows

Build:

  Linux (system packages — fast):
    pacman -S glfw assimp glm openssl boost msgpack-cxx protobuf   # or apt equivalents
    cmake -S . -B build && cmake --build build

  Linux (vcpkg — reproducible, slower first build):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/mnt/xxx/src/opengl/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build

  For debugging with sanitizers:
  cmake -S . -B build-san -DENABLE_SANITIZERS=ON && cmake --build build-san

  For faster debugging:
  cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build

  Windows (vcpkg):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=S:/src/vcpkg_win/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release

Run from build/: ./OpenGLPlayground [server --port 5001 | client --url host:port]
