# Create kio executable
add_executable(kio)
set(KIO_SOURCES
    main.cpp
    window.h
    window.cpp
    kb_input.h
    kb_input.cpp
    text_renderer.h
    text_renderer.cpp
    glyph.h
    glyph.cpp
    text_renderer_utils.h
    character.h
    character.cpp
)

# Setup xdg-shell and xdg-decoration on Wayland
if("${PLATFORM}" STREQUAL "USE_WAYLAND")
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

    set(KIO_SOURCES ${KIO_SOURCES} xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c)
endif()

# Compile vertex and fragment shaders with glslc
message(STATUS "Compiling shaders")
if(NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "Error compiling shaders, glslc not found")
endif()

add_custom_target(shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:kio>/"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" -o "$<TARGET_FILE_DIR:kio>/vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" -o "$<TARGET_FILE_DIR:kio>/frag.spv"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag"
)

add_custom_command(TARGET kio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/arial.ttf" "$<TARGET_FILE_DIR:kio>"
)

target_sources(kio PRIVATE ${KIO_SOURCES})
add_dependencies(kio shaders)
