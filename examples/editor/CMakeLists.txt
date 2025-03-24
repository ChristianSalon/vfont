# Editor scene sources
set(EDITOR_SOURCES
    main.cpp
    editor_scene.h
    editor_scene.cpp
)

# Create editor scene executable
add_executable(editor ${EDITOR_SOURCES})
target_link_libraries(editor PRIVATE vfont shared)

# Add font files and shaders to executable directory
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:editor>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/roboto/Roboto-Regular.ttf" "$<TARGET_FILE_DIR:editor>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-vert.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-frag.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-vert.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tesc.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tese.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-frag.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-vert.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-frag.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-vert.spv" "$<TARGET_FILE_DIR:editor>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-frag.spv" "$<TARGET_FILE_DIR:editor>/shaders"
)
