cmake_minimum_required(VERSION 3.12)
project(sdebugger)

set(LIBMINECRAFTPE_PATH "" CACHE FILEPATH "Path to libminecraftpe.so to link against")

include(../mod-ndk-gnustl/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 14)

include(dep/boost.cmake)
include(dep/v8.cmake)
include(dep/json.cmake)

add_library(sdebugger SHARED main.cpp inspector/InspectorManager.cpp inspector/InspectorManager.h minecraft/Crypto.h minecraft/Util.h inspector/InspectorWebSocketSession.cpp inspector/InspectorWebSocketSession.h inspector/String16.h inspector/Listener.cpp inspector/Listener.h inspector/HttpSession.cpp inspector/HttpSession.h inspector/WebSocketSession.cpp inspector/WebSocketSession.h inspector/HttpSessionHandler.h inspector/InspectorServer.cpp inspector/InspectorServer.h extra/StdPolyfills.h extra/StdPolyfills.cpp minecraft/Cohtml.h cohtml/TCPServerImpl.cpp cohtml/TCPServerImpl.h cohtml/CohtmlHooks.cpp minecraft/FilePathManager.h main.h minecraft/Keyboard.h)
target_link_libraries(sdebugger ${LIBMINECRAFTPE_PATH})
target_link_libraries(sdebugger boost v8 nlohmann_json)