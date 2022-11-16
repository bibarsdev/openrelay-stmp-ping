# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10.0)

project(openrelay-stmp-ping)

include_directories(include)

file(GLOB SourceFiles src/*.c)

add_executable(ping-smtp ${SourceFiles})
target_link_libraries(ping-smtp ws2_32)