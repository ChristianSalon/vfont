cmake_minimum_required(VERSION 3.25)

# Create executable
add_executable(vfont-demo)
set(VFONT_DEMO_SOURCES
    main.cpp
    scene.h
    scene.cpp
    demo_scene.h
    demo_scene.cpp
    editor_scene.h
    editor_scene.cpp
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

    set(VFONT_DEMO_SOURCES ${VFONT_DEMO_SOURCES} xdg-shell-client-protocol.h xdg-shell-protocol.c xdg-decoration-client-protocol.h xdg-decoration-protocol.c)
endif()

# Compile vertex and fragment shaders with glslc
message(STATUS "Compiling shaders")
if(NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "Error compiling shaders, glslc not found")
endif()

add_custom_target(shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vfont-demo>/"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" -o "$<TARGET_FILE_DIR:vfont-demo>/vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" -o "$<TARGET_FILE_DIR:vfont-demo>/frag.spv"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag"
)

# Add font files to executable directory
add_custom_command(TARGET vfont-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/roboto/Roboto-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/jersey10/Jersey10-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/crimsontext/CrimsonText-Italic.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/robotomono/RobotoMono-Bold.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/notosansjp/NotoSansJP-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/notoemoji/NotoEmoji.ttf" "$<TARGET_FILE_DIR:vfont-demo>"
)

target_sources(vfont-demo PRIVATE ${VFONT_DEMO_SOURCES})
add_dependencies(vfont-demo shaders)
