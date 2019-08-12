set(target useraudio)
file(GLOB SRC_LIST "src/*.c")
set(src ${SRC_LIST})

include_directories(include)

add_library(${target} SHARED ${src})
set_target_properties(${target} PROPERTIES OUTPUT_NAME ${target})
install(TARGETS ${target}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)
install(FILES include/useraudioapi.h DESTINATION include)

