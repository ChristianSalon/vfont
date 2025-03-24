cmake_minimum_required(VERSION 3.25)
project(vfont VERSION 1.0)

# Enable c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set cmake modules path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(LIB_NAME vfont)
set(VFONT_HEADERS
    include/VFONT/text_renderer.h
    include/VFONT/i_vulkan_text_renderer.h
    include/VFONT/vulkan_text_renderer.h
    include/VFONT/vulkan_triangulation_text_renderer.h
    include/VFONT/vulkan_tessellation_shaders_text_renderer.h
    include/VFONT/vulkan_winding_number_text_renderer.h
    include/VFONT/vulkan_sdf_text_renderer.h
    include/VFONT/vulkan_text_renderer_decorator.h
    include/VFONT/vulkan_timed_renderer.h
    include/VFONT/glyph.h
    include/VFONT/glyph_mesh.h
    include/VFONT/glyph_compositor.h
    include/VFONT/glyph_cache.h
    include/VFONT/character.h
    include/VFONT/font.h
    include/VFONT/font_atlas.h
    include/VFONT/shaper.h
    include/VFONT/text_block.h
    include/VFONT/text_block_builder.h
    include/VFONT/text_segment.h
    include/VFONT/line_divider.h
    include/VFONT/text_align_strategy.h
    include/VFONT/tessellator.h
    include/VFONT/triangulation_tessellator.h
    include/VFONT/winding_number_tessellator.h
    include/VFONT/tessellation_shaders_tessellator.h
    include/VFONT/sdf_tessellator.h
    include/VFONT/circular_dll.h
    include/VFONT/polygon_operator.h
    include/VFONT/edge.h
    include/VFONT/curve.h
    include/VFONT/unicode.h
)
set(VFONT_SOURCES
    src/text_renderer.cpp
    src/vulkan_text_renderer.cpp
    src/vulkan_triangulation_text_renderer.cpp
    src/vulkan_tessellation_shaders_text_renderer.cpp
    src/vulkan_winding_number_text_renderer.cpp
    src/vulkan_sdf_text_renderer.cpp
    src/vulkan_text_renderer_decorator.cpp
    src/vulkan_timed_renderer.cpp
    src/glyph.cpp
    src/glyph_mesh.cpp
    src/glyph_compositor.cpp
    src/glyph_cache.cpp
    src/character.cpp
    src/font.cpp
    src/font_atlas.cpp
    src/shaper.cpp
    src/text_block.cpp
    src/text_block_builder.cpp
    src/text_segment.cpp
    src/line_divider.cpp
    src/text_align_strategy.cpp
    src/tessellator.cpp
    src/triangulation_tessellator.cpp
    src/winding_number_tessellator.cpp
    src/tessellation_shaders_tessellator.cpp
    src/sdf_tessellator.cpp
    src/circular_dll.cpp
    src/polygon_operator.cpp
    src/edge.cpp
    src/curve.cpp
    src/unicode.cpp
)

add_library(${LIB_NAME} STATIC ${VFONT_HEADERS} ${VFONT_SOURCES})

target_include_directories(${LIB_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/VFONT>
    $<INSTALL_INTERFACE:include/VFONT>
)
target_include_directories(${LIB_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Build third party libraries
add_subdirectory(thirdparty)

# Link Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)
target_include_directories(${LIB_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} PUBLIC Vulkan::Vulkan)

# Link freetype
target_link_libraries(${LIB_NAME} PRIVATE freetype)

# Link CDT
target_link_libraries(${LIB_NAME} PRIVATE CDT::CDT)

# Link glm
target_link_libraries(${LIB_NAME} PUBLIC glm::glm)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Link harfbuzz
target_link_libraries(${LIB_NAME} PUBLIC harfbuzz)

# Compile shaders with glslc
message(STATUS "vfont: Compiling shaders")
if(NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "vfont: Error compiling shaders, glslc not found")
endif()

add_custom_target(shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.vert" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle-vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/triangle-frag.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.vert" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/curve-vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.tesc" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/curve-tesc.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.tese" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/curve-tese.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.frag" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/curve-frag.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/winding_number.vert" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/winding_number-vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/winding_number.frag" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/winding_number-frag.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sdf.vert" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/sdf-vert.spv"
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sdf.frag" -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/sdf-frag.spv"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/triangle.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.tesc"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.tese"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/curve.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/winding_number.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/winding_number.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sdf.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sdf.frag"
)

add_dependencies(${LIB_NAME} shaders)

# Build documentation if specified
option(BUILD_DOCS "Build documentation" OFF)
if(BUILD_DOCS)
    message(STATUS "vfont: Building documentation")

    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs)

        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "vfont: Generating documentation"
            VERBATIM
        )

        add_dependencies(${LIB_NAME} docs)
    else()
        message(WARNING "vfont: Could not build documentation, please install doxygen")
    endif()
endif()

# Build examples if specified
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    message(STATUS "vfont: Building examples")
    add_subdirectory(examples)
endif()
