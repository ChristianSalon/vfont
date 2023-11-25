cmake_minimum_required(VERSION 3.25)
project(kio VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/")

set(VFONT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vfont")
set(LIBS "vfont")
set(INCLUDES ${CMAKE_CURRENT_BINARY_DIR})
set(DEFINES "")

# Set platform specific stuff
if(WIN32)
    set(DEFINES ${DEFINES} USE_WIN32 NOMINMAX UNICODE VK_USE_PLATFORM_WIN32_KHR)
    set(PLATFORM "USE_WIN32")
elseif(UNIX AND NOT APPLE)
    if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "USE_WAYLAND")
        # Use Wayland by default on linux if PLATFORM is not defined
        find_package(Wayland)

        # xkbcommon - library for handling keyboard input on Wayland
        find_package(Xkbcommon)

        if(Wayland_FOUND AND Xkbcommon_FOUND)
            set(DEFINES ${DEFINES} USE_WAYLAND VK_USE_PLATFORM_WAYLAND_KHR ${Xkbcommon_DEFINES})
            set(LIBS ${LIBS} ${Wayland_client_LIBRARIES} ${Xkbcommon_LIBRARIES} -lrt)
            set(INCLUDES ${INCLUDES} ${Wayland_client_INCLUDE_DIRS} ${Xkbcommon_INCLUDE_DIRS})
            set(PLATFORM "USE_WAYLAND")
        elseif("${PLATFORM}" STREQUAL "USE_WAYLAND")
            message(WARNING "Try installing wayland, wayland-protocols and xkbcommon")
            message(FATAL_ERROR "Selected platform is not supported")
        elseif(NOT DEFINED PLATFORM)
            message(WARNING "To build for Wayland, try installing wayland, wayland-protocols and xkbcommon")
            message(WARNING "Platform Wayland is not supported, trying X11")
        endif()
    endif()

    if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "USE_X11")
        find_package(X11 REQUIRED)
        if(X11_FOUND)
            set(DEFINES ${DEFINES} USE_X11 VK_USE_PLATFORM_XLIB_KHR)
            set(LIBS ${LIBS} X11)
            set(PLATFORM "USE_X11")
        elseif("${PLATFORM}" STREQUAL "USE_X11")
            message(FATAL_ERROR "Selected platform is not supported")
        elseif(NOT DEFINED PLATFORM)
            message(FATAL_ERROR "Platform X11 is not supported")
        endif()
    endif()
endif()
message(STATUS "Selected platform is ${PLATFORM}")

# Find doxygen and build documentation if specified
option(BUILD_DOC "Build documentation" OFF)

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
