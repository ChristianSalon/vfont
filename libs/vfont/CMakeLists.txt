cmake_minimum_required(VERSION 3.25)
project(vfont VERSION 1.0)

set(LIB_NAME vfont)
set(VFONT_HEADERS
    include/VFONT/text_renderer.h
    include/VFONT/text_renderer_utils.h
    include/VFONT/glyph.h
    include/VFONT/character.h
    include/VFONT/font.h
    include/VFONT/text_block.h
    include/VFONT/glyph_compositor.h
)
set(VFONT_SOURCES
    src/text_renderer.cpp
    src/glyph.cpp
    src/character.cpp
    src/font.cpp
    src/text_block.cpp
    src/glyph_compositor.cpp
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

# link Vulkan
target_include_directories(${LIB_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${LIB_NAME} PRIVATE Vulkan::Vulkan)

# link freetype
target_link_libraries(${LIB_NAME} PUBLIC freetype)

# link CDT
target_link_libraries(${LIB_NAME} PRIVATE CDT::CDT)

# link glm
target_link_libraries(${LIB_NAME} PUBLIC glm::glm)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
