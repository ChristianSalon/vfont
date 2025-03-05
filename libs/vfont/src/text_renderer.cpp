/**
 * @file text_renderer.cpp
 * @author Christian Saloň
 */

#include "text_renderer.h"

namespace vft {

/**
 * @brief Initializes text renderer
 *
 * @param physicalDevice Vulkan physical device
 * @param logicalDevice Vulkan logical device
 * @param commandPool Vulkan command pool
 * @param graphicsQueue Vulkan graphics queue
 */
void TextRenderer::initialize() {
    this->_cache = std::make_shared<GlyphCache>();
}

/**
 * @brief Must be called to destroy vulkan buffers
 */
void TextRenderer::destroy() {
    this->_tessellator.reset();
}

/**
 * @brief Add text block for rendering
 *
 * @param text Text block to render
 */
void TextRenderer::add(std::shared_ptr<TextBlock> text) {
    this->_textBlocks.push_back(text);
    text->onTextChange = [this]() { this->update(); };
}

void TextRenderer::setUniformBuffers(UniformBufferObject ubo) {
    this->_ubo = ubo;
}

void TextRenderer::setViewportSize(unsigned int width, unsigned int height) {
    this->_viewportWidth = width;
    this->_viewportHeight = height;
}

void TextRenderer::setCache(std::shared_ptr<GlyphCache> cache) {
    this->_cache = cache;
}

}  // namespace vft
