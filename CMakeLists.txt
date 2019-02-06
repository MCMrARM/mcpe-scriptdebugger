cmake_minimum_required(VERSION 3.12)
project(sdebugger)

set(LIBMINECRAFTPE_PATH "" CACHE FILEPATH "Path to libminecraftpe.so to link against")

include(../mod-ndk-gnustl/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 14)

include(dep/asio.cmake)
include(dep/websocket.cmake)
include(dep/v8.cmake)

add_library(sdebugger SHARED main.cpp inspector/InspectorManager.cpp inspector/InspectorManager.h inspector/WebSockets.h minecraft/Crypto.h minecraft/Util.h inspector/InspectorWebSocketChannel.cpp inspector/InspectorWebSocketChannel.h inspector/String16.h inspector/InspectorWebSocketManager.cpp inspector/InspectorWebSocketManager.h)
target_link_libraries(sdebugger ${LIBMINECRAFTPE_PATH})
target_link_libraries(sdebugger asio websocket v8)