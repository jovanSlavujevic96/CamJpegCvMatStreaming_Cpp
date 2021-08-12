cmake_minimum_required(VERSION 3.8)
project (CameraSocketStreaming_Cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

if (MSVC)
	set(CMAKE_MODULE_PATH "$ENV{OPENCV_DIR};${CMAKE_MODULE_PATH}")
endif()
find_package(OpenCV REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

include_directories("${OpenCV_INCLUDE_DIRS}")
link_libraries("${OpenCV_LIBS}")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Multicast")
if (MSVC)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/RtspUnicastServer_Win")
endif()
