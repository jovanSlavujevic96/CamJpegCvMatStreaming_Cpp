project(RTSP_CameraStream_Cpp)

add_executable(clientIPv4 "${CMAKE_CURRENT_SOURCE_DIR}/ipv4_client.cpp")
add_executable(serverIPv4 "${CMAKE_CURRENT_SOURCE_DIR}/ipv4_server.cpp")
add_executable(clientIPV6 "${CMAKE_CURRENT_SOURCE_DIR}/ipv6_client.cpp")
add_executable(serverIPV6 "${CMAKE_CURRENT_SOURCE_DIR}/ipv6_server.cpp")

if (MSVC)
	target_compile_definitions(clientIPv4 PUBLIC -DWIN)
	target_compile_definitions(serverIPv4 PUBLIC -DWIN)
	target_compile_definitions(clientIPV6 PUBLIC -DWIN)
	target_compile_definitions(serverIPV6 PUBLIC -DWIN)
endif (MSVC)
