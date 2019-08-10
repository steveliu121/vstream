CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(example)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

link_directories($ENV{ROOT_DIR}/.yarina/share/lib)
include_directories($ENV{ROOT_DIR}/.yarina/share/include)

set(depend_lib userstream)
file(GLOB SRC_LIST "*.c")

include(pi_videocore.cmake)

foreach(src ${SRC_LIST})
	get_filename_component(target ${src} NAME_WE)
	add_executable(${target} ${src})
	target_link_libraries(${target} ${depend_lib})
	install(TARGETS ${target} RUNTIME DESTINATION bin)
endforeach(src)
