#!/bin/bash
set -e

cd "$(dirname "$0")/.."

mkdir -p build
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++"

cmake --build . --config Release

# Build packages
cpack -G RPM
cpack -G DEB
cpack -G TGZ

echo "Packages built in $(pwd)"