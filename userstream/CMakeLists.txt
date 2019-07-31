CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(userstream)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(target userstream)
file(GLOB SRC_LIST "src/*.c")
set(src ${SRC_LIST})

include(pi_videocore.cmake)
include_directories(include)

add_library(${target} SHARED ${src})
set_target_properties(${target} PROPERTIES OUTPUT_NAME ${target})
install(TARGETS ${target}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)
install(FILES include/uservideoapi.h DESTINATION include)
