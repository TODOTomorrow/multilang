cmake_minimum_required(VERSION 2.8)

project(mll-js)
file(GLOB_RECURSE sources "*.cpp")
include_directories(include)
include_directories(include/duktape)

link_directories(${CMAKE_CURRENT_LIST_DIR}/libs/)

add_library(mll-js SHARED  ${sources})
target_link_libraries(mll-js duktape)


