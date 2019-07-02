set(target userv4l2)
FILE(GLOB SRC_LIST "src/*.c")
set(src ${SRC_LIST})

add_library(${target} SHARED ${src})
set_target_properties(${target} PROPERTIES OUTPUT_NAME ${target})
install(TARGETS ${target}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)
