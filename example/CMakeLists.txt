CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(example)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

LINK_DIRECTORIES($ENV{ROOT_DIR}/.yarina/share/lib)
INCLUDE_DIRECTORIES($ENV{ROOT_DIR}/.yarina/share/include)

set(depend_lib userstream)
FILE(GLOB SRC_LIST "*.c")

FOREACH(src ${SRC_LIST})
	get_filename_component(target ${src} NAME_WE)
	add_executable(${target} ${src})
	target_link_libraries(${target} ${depend_lib})
	install(TARGETS ${target} RUNTIME DESTINATION bin)
ENDFOREACH(src)
