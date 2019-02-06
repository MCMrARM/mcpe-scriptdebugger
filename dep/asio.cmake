include(ExternalProject)

ExternalProject_Add(
        asio_ext
        URL "https://github.com/chriskohlhoff/asio/archive/asio-1-12-2.zip"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(asio_ext SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR}/asio/include)
add_library(asio INTERFACE IMPORTED)
add_dependencies(asio asio_ext)
set_property(TARGET asio PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR}/asio/include)