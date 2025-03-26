cmake_minimum_required(VERSION 3.26.0)
project(NeoSC VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer_ext CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
set (OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

add_subdirectory(src)
add_subdirectory(src/sc)
