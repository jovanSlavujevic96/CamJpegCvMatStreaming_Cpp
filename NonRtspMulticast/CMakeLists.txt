cmake_minimum_required(VERSION 3.10.2)
project(RTSP_CameraStream_Cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH "$ENV{OPENCV_DIR};${CMAKE_MODULE_PATH}")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(IPv4CameraMulticast) #Non RTSP
add_subdirectory(IPv6CameraMulticast) #Non RTSP
