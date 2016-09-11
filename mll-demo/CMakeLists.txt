cmake_minimum_required(VERSION 2.8)

project(mll-demo)
file(GLOB_RECURSE sources "*.cpp")
include_directories(include)

add_executable(mll-demo ${sources})
target_link_libraries(mll-demo mll)
