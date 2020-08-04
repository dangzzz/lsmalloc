cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
project(lsmalloc)



include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src lsmalloc_src)

SET (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib)

add_library(lsmalloc_static STATIC ${lsmalloc_src})

target_compile_options(lsmalloc_static PRIVATE -Wall -O3 -pthread)

set_target_properties(lsmalloc_static PROPERTIES OUTPUT_NAME "lsmalloc")

GET_TARGET_PROPERTY (OUTPUT_VALUE lsmalloc_static OUTPUT_NAME)


