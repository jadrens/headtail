#!/bin/bash
set -e

cd "$(dirname "$0")/.."

mkdir -p build-windows
cd build-windows

# Cross-compile for Windows using MinGW
cmake .. \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
    -DCMAKE_RC_COMPILER=i686-w64-mingw32-wrc \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++"

cmake --build . --config Release

# Build Windows installer using WiX
cpack -G WIX