set(SHARED_SOURCES
    scene.h
    scene.cpp
    window.h
    window.cpp
    kb_input.h
    kb_input.cpp
    base_camera.h
    base_camera.cpp
    ortho_camera.h
    ortho_camera.cpp
    perspective_camera.h
    perspective_camera.cpp
)

add_library(shared STATIC)
target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(shared PUBLIC vfont)

# Set platform specific stuff
if(WIN32)
    add_compile_definitions(USE_WIN32 NOMINMAX UNICODE VK_USE_PLATFORM_WIN32_KHR)
    set(PLATFORM "USE_WIN32")
elseif(UNIX AND NOT APPLE)
    if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "USE_WAYLAND")
        # Use Wayland by default on linux if PLATFORM is not defined
        find_package(Wayland)
        # xkbcommon - library for handling keyboard input on Wayland
        find_package(Xkbcommon)

        if(Wayland_FOUND AND Xkbcommon_FOUND)
            add_compile_definitions(USE_WAYLAND VK_USE_PLATFORM_WAYLAND_KHR ${Xkbcommon_DEFINES})
            target_link_libraries(shared ${Wayland_client_LIBRARIES} ${Xkbcommon_LIBRARIES} -lrt)
            target_include_directories(shared ${Wayland_client_INCLUDE_DIRS} ${Xkbcommon_INCLUDE_DIRS})

            # Setup xdg-shell and xdg-decoration
            add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell-client-protocol.h
                COMMAND ${Wayland_SCANNER} client-header ${Wayland_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell-client-protocol.h
            )
            add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell-protocol.c
                COMMAND ${Wayland_SCANNER} private-code ${Wayland_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell-protocol.c
            )
            add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-client-protocol.h
                COMMAND ${Wayland_SCANNER} client-header ${Wayland_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-client-protocol.h
            )
            add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-protocol.c
                COMMAND ${Wayland_SCANNER} private-code  ${Wayland_PROTOCOLS_DIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml ${CMAKE_CURRENT_SOURCE_DIR}/xdg-decoration-protocol.c
            )
            set(SHARED_SOURCES ${SHARED_SOURCES} xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c)

            set(PLATFORM "USE_WAYLAND")
        elseif("${PLATFORM}" STREQUAL "USE_WAYLAND")
            message(WARNING "vfont: Try installing wayland, wayland-protocols and xkbcommon")
            message(FATAL_ERROR "vfont: Selected platform is not supported")
        elseif(NOT DEFINED PLATFORM)
            message(WARNING "vfont: To build for Wayland, try installing wayland, wayland-protocols and xkbcommon")
            message(WARNING "vfont: Platform Wayland is not supported, trying X11")
        endif()
    endif()

    if(NOT DEFINED PLATFORM OR "${PLATFORM}" STREQUAL "USE_X11")
        find_package(X11 REQUIRED)

        if(X11_FOUND)
            add_compile_definitions(USE_X11 VK_USE_PLATFORM_XLIB_KHR)
            target_link_libraries(shared X11)
            set(PLATFORM "USE_X11")
        elseif("${PLATFORM}" STREQUAL "USE_X11")
            message(FATAL_ERROR "vfont: Selected platform is not supported")
        elseif(NOT DEFINED PLATFORM)
            message(FATAL_ERROR "vfont: Platform X11 is not supported")
        endif()
    endif()
endif()
message(STATUS "vfont: Selected platform is ${PLATFORM}")

# Set source files for target shared
target_sources(shared PRIVATE ${SHARED_SOURCES})
