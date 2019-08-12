CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(userstream)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_subdirectory(video)
add_subdirectory(audio)
