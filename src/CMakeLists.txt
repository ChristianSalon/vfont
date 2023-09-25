# Create kio executable
add_executable(kio)
target_sources(kio PRIVATE main.cpp window.h window.cpp kb_input.h kb_input.cpp)

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

add_dependencies(kio shaders)
