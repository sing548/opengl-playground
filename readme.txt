This is a small passion/learning project for graphics programming using C++ and OpenGL.
Currently mostly the architecture containing a basic pipeline handling rendering, shaders and gameplay systems and networking using Valve's GameNetworkingSockets.

What's implemented so far:
- Rendering pipeline with shader handling, sRGB-correct color, bloom
- Model loading (assimp)
- Client/server networking (Valve's GameNetworkingSockets)
- Client-side prediction with reconciliation
- Snapshot interpolation (cubic Hermite) behind a rate-corrected clock
- Server-side input de-jitter queue
- Message serialization (msgpack)
- Gameplay systems on a phase-based scheduler
- Procedural terrain
- CMake build with optional vcpkg, Linux & Windows
- Debug tooling: Dear ImGui panel with live metrics and runtime fake lag/loss/jitter injection
- CMake build with optional vcpkg, Linux & Windows

Build:

  Linux (system packages — fast):
    pacman -S glfw assimp glm openssl boost msgpack-cxx   # or apt equivalents
    cmake -S . -B build && cmake --build build

  Linux (vcpkg — reproducible, slower first build):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/mnt/xxx/src/opengl/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build

  For debugging with sanitizers:
  cmake -S . -B build-san -DENABLE_SANITIZERS=ON && cmake --build build-san

  For faster debugging:
  cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build

  Release version:
  cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release && cmake --build build-release

  Windows (vcpkg):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=S:/src/vcpkg_win/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release

Run from build/: ./OpenGLPlayground [server --port 5001 | client --url host:port]
