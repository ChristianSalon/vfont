cmake_minimum_required(VERSION 3.25)
project(kio VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

set(VFONT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vfont")
set(LIBS "vfont")
set(INCLUDES ${CMAKE_CURRENT_BINARY_DIR})
set(DEFINES "")

# Add target platform defines
if(WIN32)
    set(DEFINES ${DEFINES} USE_WIN32 NOMINMAX UNICODE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX AND NOT APPLE)
    # Default use X11 on linux
    find_package(X11 REQUIRED)
    if(X11_FOUND)
	    set(DEFINES ${DEFINES} USE_X11 VK_USE_PLATFORM_XLIB_KHR)
        set(LIBS ${LIBS} X11)
    endif()
endif()

# Find doxygen and build documentation if specified
option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)
if(BUILD_DOC AND DOXYGEN_FOUND)
    set(DOXYGEN_EXTRACT_PRIVATE "YES")
    set(DOXYGEN_ENABLE_PREPROCESSING "NO")

    # kio documentation
    message(STATUS "Generating documentation")
    set(DOXYGEN_PROJECT_NAME "kio")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc/kio")
    doxygen_add_docs(
        kio_doc
        ALL
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    # vfont documentation
    set(DOXYGEN_PROJECT_NAME "vfont")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc/vfont")
    doxygen_add_docs(
        vfont_doc
        ALL
        "${VFONT_PROJECT_DIR}/src"
    )
elseif(BUILD_DOC AND NOT DOXYGEN_FOUND)
    message(WARNING "Could not build documentation, please install doxygen")
endif()

# Vulkan initialization
find_package(Vulkan REQUIRED COMPONENTS glslc)
set(INCLUDES ${INCLUDES} ${Vulkan_INCLUDE_DIRS})
set(LIBS ${LIBS} Vulkan::Vulkan)

add_compile_definitions(${DEFINES})

add_subdirectory("src")
add_subdirectory("libs/vfont")

target_include_directories(kio PRIVATE ${INCLUDES})
target_link_libraries(kio PRIVATE ${LIBS})
