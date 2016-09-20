cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO,"true")
set(CMAKE_CXX_FLAGS_DEBUG,"true")

project(mll-demo)
file(GLOB_RECURSE sources "*.cpp" "*.h")
include_directories(include)

add_library(mll SHARED ${sources})
target_link_libraries(mll dl)
