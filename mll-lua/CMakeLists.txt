cmake_minimum_required(VERSION 2.8)

project(mll-lua)
file(GLOB_RECURSE sources "*.cpp")
include_directories(include)
include_directories(include/lua)

link_directories(${CMAKE_CURRENT_LIST_DIR}/libs/)
message(${CMAKE_CURRENT_LIST_DIR}/libs/)
add_library(mll-lua SHARED ${sources})

target_link_libraries(mll-lua lua)
