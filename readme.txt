Build:

  Linux (system packages — fast):
    pacman -S glfw assimp glm openssl boost msgpack-cxx   # or apt equivalents
    cmake -S . -B build && cmake --build build

  Linux (vcpkg — reproducible, slower first build):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/mnt/xxx/src/opengl/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build

  Windows (vcpkg):
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=S:/src/vcpkg_win/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release

Run from build/: ./OpenGLPlayground [server --port 5001 | client host:port]
