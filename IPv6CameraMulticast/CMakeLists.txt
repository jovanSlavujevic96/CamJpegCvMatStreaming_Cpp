add_executable(clientIPV6 client.cpp)
target_link_libraries(clientIPV6 PUBLIC ${OpenCV_LIBS})

add_executable(serverIPV6 server.cpp)
target_link_libraries(serverIPV6 PUBLIC ${OpenCV_LIBS})
