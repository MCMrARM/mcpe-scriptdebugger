include(ExternalProject)

ExternalProject_Add(
        v8_ext
        URL "https://chromium.googlesource.com/v8/v8.git/+archive/refs/heads/5.9.205/include.tar.gz"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(v8_ext SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR})
add_library(v8 INTERFACE IMPORTED)
add_dependencies(v8 v8_ext)
set_property(TARGET v8 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR})