cmake_minimum_required(VERSION 2.8)

project(mll-demo)
file(GLOB_RECURSE sources "*.cpp" "*.h")
include_directories(include)

add_library(mll SHARED ${sources})
target_link_libraries(mll dl)
