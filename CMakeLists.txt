cmake_minimum_required(VERSION 3.0)
project(switch_proxy)

#程序名
set(OUT_NAME switch_proxy_server)

#debug或release模式
if(CMAKE_BUILD_TYPE MATCHES "debug")
    message("Build debug...")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -g -O0 -std=c++11")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-variable -g -O3 -std=c++11")   
endif()

#遍历主目录源文件，删除cmake自动生成的main源文件
file(GLOB_RECURSE  SOURCE_FILES "*.cpp")
file(GLOB_RECURSE  DEBUG_REMOVE_FILE "debug/*")
file(GLOB_RECURSE  RELEASE_REMOVE_FILE "release/*")
list(REMOVE_ITEM SOURCE_FILES ${DEBUG_REMOVE_FILE} ${RELEASE_REMOVE_FILE})

#依赖包含 
include_directories(../)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

#可执行程序
add_executable(${OUT_NAME} ${SOURCE_FILES})

#连接库
set(LINK_LIST pthread rt boost_system-mt)
target_link_libraries(${OUT_NAME} ${LINK_LIST})
