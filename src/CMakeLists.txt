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
    benchmark_scene.h
    benchmark_scene.cpp
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

# Add font files and vfont shaders to executable directory
add_custom_command(TARGET vfont-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/roboto/Roboto-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/jersey10/Jersey10-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/crimsontext/CrimsonText-Italic.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/robotomono/RobotoMono-Bold.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/notosansjp/NotoSansJP-Regular.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/assets/notoemoji/NotoEmoji.ttf" "$<TARGET_FILE_DIR:vfont-demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/triangle-vert.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/triangle-frag.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/curve-vert.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/curve-tesc.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/curve-tese.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/curve-frag.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/winding_number-vert.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/winding_number-frag.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/sdf-vert.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/libs/vfont/shaders/sdf-frag.spv" "$<TARGET_FILE_DIR:vfont-demo>/shaders"
)

target_sources(vfont-demo PRIVATE ${VFONT_DEMO_SOURCES})
