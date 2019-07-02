CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(vstream)

LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libuserv4l2/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_subdirectory(libuserv4l2)
add_subdirectory(test)
