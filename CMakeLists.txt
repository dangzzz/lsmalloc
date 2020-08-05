cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
project(lsmalloc)

#make时显式输出执行的命令
set(CMAKE_VERBOSE_MAKEFILE ON)

#指定库的输出位置
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin)

#添加头文件搜索目录,相当于-I
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

#将src目录下所有的源文件加入列表lsmalloc_src中
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src lsmalloc_src)

#添加目标静态库文件lsmalloc_static,生成所用的源文件是lsmalloc_src
add_library(lsmalloc_static STATIC ${lsmalloc_src})

#为lsmalloc_static添加编译器flag
target_compile_options(lsmalloc_static PRIVATE -std=gnu99 -g3 -Wall -pipe -funroll-loops -O0 -D_GNU_SOURCE -D_REENTRANT)
target_link_libraries(lsmalloc_static pthread)
target_link_libraries(lsmalloc_static pmem)
#设置lsmalloc_static输出时叫lsmalloc
set_target_properties(lsmalloc_static PROPERTIES OUTPUT_NAME "lsmalloc")

#warning一下hpy
message(AUTHOR_WARNING "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n目前生成的Makefile功能是把src目录下的所有源文件编译打包成一个静态库放在lib文件夹下.\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")


add_executable(sample_test ${CMAKE_CURRENT_LIST_DIR}/test/integration/sample_test.c)
target_compile_options(sample_test PRIVATE -std=gnu99 -g3 -Wall -O0 -D_GNU_SOURCE -D_REENTRANT)
target_link_libraries(sample_test lsmalloc_static)

add_executable(nofree ${CMAKE_CURRENT_LIST_DIR}/test/integration/nofree.cpp)
target_compile_options(nofree PRIVATE -std=c++11 -g3 -Wall -O0 -D_GNU_SOURCE -D_REENTRANT)
target_link_libraries(nofree lsmalloc_static)


enable_testing()
add_test(sample_test ${EXECUTABLE_OUTPUT_PATH}/sample_test)
add_test(nofree ${EXECUTABLE_OUTPUT_PATH}/nofree 2)
