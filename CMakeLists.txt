cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
project(lsmalloc)

#make时显式输出执行的命令
set(CMAKE_VERBOSE_MAKEFILE ON)

#指定库的输出位置
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib)

#添加头文件搜索目录,相当于-I
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

#将src目录下所有的源文件加入列表lsmalloc_src中
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src lsmalloc_src)

#添加目标静态库文件lsmalloc_static,生成所用的源文件是lsmalloc_src
add_library(lsmalloc_static STATIC ${lsmalloc_src})

#为lsmalloc_static添加编译器flag
target_compile_options(lsmalloc_static PRIVATE -Wall -O3 -pthread)

#设置lsmalloc_static输出时叫lsmalloc
set_target_properties(lsmalloc_static PROPERTIES OUTPUT_NAME "lsmalloc")

#warning一下hpy
message(AUTHOR_WARNING "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n目前生成的Makefile功能是把src目录下的所有源文件编译打包成一个静态库放在lib文件夹下.\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")


