To start: 
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/mnt/xxx/src/opengl/vcpkg/scripts/buildsystems/vcpkg.cmake
cd build
make


windows:
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=S:/src/vcpkg_win/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release