cmake_minimum_required(VERSION 3.10)

project("GlassEngine")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

add_subdirectory(Tools)
add_subdirectory(Application)
add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(GlassRenderer)