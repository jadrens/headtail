cmake_minimum_required(VERSION 3.15)
project(headtail VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(headtail main.cpp)

target_compile_options(headtail PRIVATE -Wall -Wextra)

install(TARGETS headtail DESTINATION bin)