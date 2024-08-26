cmake_minimum_required(VERSION 3.5.0)
project(NeoSC VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL2 REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(src)
add_subdirectory(src/sc)
