cmake_minimum_required(VERSION 2.8)

file(REMOVE_RECURSE mll-demo/include/mll)
file(REMOVE_RECURSE mll-lua/include/mll)

execute_process(COMMAND ln -fs ${CMAKE_CURRENT_LIST_DIR}/mll/include/mll/ ${CMAKE_CURRENT_LIST_DIR}/mll-demo/include/mll)
execute_process(COMMAND ln -fs ${CMAKE_CURRENT_LIST_DIR}/mll/include/mll/ ${CMAKE_CURRENT_LIST_DIR}/mll-lua/include/mll)
execute_process(COMMAND ln -fs ${CMAKE_CURRENT_LIST_DIR}/mll/include/mll/ ${CMAKE_CURRENT_LIST_DIR}/mll-js/include/mll)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb")

add_subdirectory(mll)
add_subdirectory(mll-lua)
add_subdirectory(mll-js)
add_subdirectory(mll-demo)
