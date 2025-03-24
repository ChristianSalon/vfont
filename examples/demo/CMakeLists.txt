# Demo scene sources
set(DEMO_SOURCES
    main.cpp
    demo_scene.h
    demo_scene.cpp
)

# Create demo scene executable
add_executable(demo ${DEMO_SOURCES})
target_link_libraries(demo PRIVATE vfont shared)

# Add font files and shaders to executable directory
add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/roboto/Roboto-Regular.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/jersey10/Jersey10-Regular.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/crimsontext/CrimsonText-Italic.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/robotomono/RobotoMono-Bold.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/notosansjp/NotoSansJP-Regular.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/notoemoji/NotoEmoji.ttf" "$<TARGET_FILE_DIR:demo>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-vert.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-frag.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-vert.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tesc.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tese.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-frag.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-vert.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-frag.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-vert.spv" "$<TARGET_FILE_DIR:demo>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-frag.spv" "$<TARGET_FILE_DIR:demo>/shaders"
)