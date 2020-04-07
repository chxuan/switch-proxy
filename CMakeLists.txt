cmake_minimum_required(VERSION 3.0)
project(switch_proxy)

#程序名
set(OUT_NAME switch_proxy_server)

#编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -g -O2 -std=c++11")   

#遍历主目录源文件，删除cmake自动生成的main源文件
file(GLOB_RECURSE  SOURCE_FILES "*.cpp")
file(GLOB_RECURSE  DEBUG_REMOVE_FILE "build/*")
list(REMOVE_ITEM SOURCE_FILES ${DEBUG_REMOVE_FILE} ${RELEASE_REMOVE_FILE})

#依赖第三方库头文件
include_directories(../)
include_directories($ENV{BOOST_INCLUDE_PATH})

#依赖第三方库lib文件
link_directories($ENV{BOOST_LIB_PATH})

#可执行程序
add_executable(${OUT_NAME} ${SOURCE_FILES})

#连接库
set(LINK_LIST pthread rt boost_system-mt)
target_link_libraries(${OUT_NAME} ${LINK_LIST})
