cmake_minimum_required(VERSION 3.5.0)
project(NeoSC VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL REQUIRED)
find_package(opengl REQUIRED)

add_subdirectory(src)

