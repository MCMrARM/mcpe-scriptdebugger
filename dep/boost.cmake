include(ExternalProject)

ExternalProject_Add(
        boost_ext
        URL "https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz"
        CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_DIR "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(boost_ext SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR})
add_library(boost INTERFACE IMPORTED)
add_dependencies(boost boost_ext)
set_property(TARGET boost PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR})