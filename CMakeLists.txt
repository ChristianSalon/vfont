cmake_minimum_required(VERSION 3.16)
project(kio VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

set(VFONT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/vfont)

# Add target platform definitions
if(WIN32)
	add_compile_definitions(USE_WIN32 UNICODE)
elseif(UNIX AND NOT APPLE)
	add_compile_definitions(USE_X11)
endif()

# Find doxygen and build documentation if specified
option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)

if(BUILD_DOC AND DOXYGEN_FOUND)
    # kio documentation
    set(DOXYGEN_PROJECT_NAME kio)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/kio)
    doxygen_add_docs(
        kio_doc
        ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # vfont documentation
    set(DOXYGEN_PROJECT_NAME vfont)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/vfont)
    doxygen_add_docs(
        vfont_doc
        ALL
        ${VFONT_PROJECT_DIR}/src
    )
elseif(BUILD_DOC AND NOT DOXYGEN_FOUND)
    message(WARNING "Could not build documentation, please install doxygen")
endif()

add_subdirectory(src)
add_subdirectory(libs/vfont)

target_link_libraries(kio PRIVATE vfont)
