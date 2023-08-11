project(vfont VERSION 1.0)

add_library(vfont STATIC)

target_sources(vfont PRIVATE src/main.cpp)

target_include_directories(vfont PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/VFONT>
    $<INSTALL_INTERFACE:include/VFONT>
)

target_include_directories(vfont PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
