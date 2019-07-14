CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(aacencoder)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES($ENV{ROOT_DIR}/.yarina/share/include)
LINK_DIRECTORIES($ENV{ROOT_DIR}/.yarina/share/lib)

set(target aacencoder)
set(depend_lib fdk-aac)
FILE(GLOB SRC_LIST "src/*.c")
set(src ${SRC_LIST})

add_library(${target} SHARED ${src})
target_link_libraries(${target} ${depend_lib})
set_target_properties(${target} PROPERTIES OUTPUT_NAME ${target})
install(TARGETS ${target}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)
install(FILES include/aacenc.h DESTINATION include)
