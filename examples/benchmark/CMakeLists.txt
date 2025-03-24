# Benchmark scene sources
set(BENCHMARK_SOURCES
    main.cpp
    benchmark_scene.h
    benchmark_scene.cpp
)

# Create benchmark scene executable
add_executable(benchmark ${BENCHMARK_SOURCES})
target_link_libraries(benchmark PRIVATE vfont shared)

# Add font files and shaders to executable directory
add_custom_command(TARGET benchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:benchmark>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/assets/roboto/Roboto-Regular.ttf" "$<TARGET_FILE_DIR:benchmark>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-vert.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/triangle-frag.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-vert.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tesc.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-tese.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/curve-frag.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-vert.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/winding_number-frag.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-vert.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/shaders/sdf-frag.spv" "$<TARGET_FILE_DIR:benchmark>/shaders"
)
