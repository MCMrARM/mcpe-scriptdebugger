include(ExternalProject)

ExternalProject_Add(
        websocket_ext
        URL "https://gitlab.com/eidheim/Simple-WebSocket-Server/-/archive/v2.0.0-rc4/Simple-WebSocket-Server-v2.0.0-rc4.zip"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(websocket_ext SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR})
add_library(websocket INTERFACE IMPORTED)
add_dependencies(websocket websocket_ext)
set_property(TARGET websocket PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR})