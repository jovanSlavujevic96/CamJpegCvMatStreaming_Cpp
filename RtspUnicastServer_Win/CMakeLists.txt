project (RtspUnicastServer_Win)

file(GLOB RTSP_UCAST_SRV_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

file(GLOB RTSP_UCAST_SRV_SRC_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(RtspUnicastServer STATIC 
	"${RTSP_UCAST_SRV_HEADER_FILES}"
	"${RTSP_UCAST_SRV_SRC_FILES}"
)

add_executable(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")
target_link_libraries(${PROJECT_NAME} PRIVATE RtspUnicastServer)
