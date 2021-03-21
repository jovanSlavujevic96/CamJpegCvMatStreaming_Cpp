cmake_minimum_required (VERSION 3.8)
project (RtspServer)

set(CMAKE_MODULE_PATH "$ENV{OPENCV_DIR};${CMAKE_MODULE_PATH}")
find_package( OpenCV CONFIG REQUIRED )

set(CMAKE_BUILD_TYPE "Release")

include_directories( 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(libRtsp STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CRtspMaster.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CRtspClientHandler.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CRtspSession.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CStreamer.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CException.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CJpegSample.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/CWebcamCvSample.cc"
 )
 target_include_directories(libRtsp PUBLIC
	"${OpenCV_INCLUDE_DIRS}"
 )
  target_link_libraries(libRtsp PUBLIC
	"${OpenCV_LIBS}"
 )

add_executable(RtspStreamer 
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
)
target_link_libraries(RtspStreamer PUBLIC 
	libRtsp
)
