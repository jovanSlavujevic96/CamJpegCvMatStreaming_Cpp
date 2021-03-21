add_executable(clientIPv4 client.cpp)
target_link_libraries(clientIPv4 PUBLIC ${OpenCV_LIBS})

add_executable(serverIPv4 server.cpp)
target_link_libraries(serverIPv4 PUBLIC ${OpenCV_LIBS})
