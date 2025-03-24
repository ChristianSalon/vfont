/**
 * @file text_renderer.cpp
 * @author Christian Saloň
 */

#include "text_renderer.h"

namespace vft {

/**
 * @brief Initializes text renderer
 */
TextRenderer::TextRenderer() {
    this->_cache = std::make_shared<GlyphCache>();
}

/**
 * @brief Destroys text renderer
 */
TextRenderer::~TextRenderer() {
    // Destroy tessellator
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

/**
 * @brief Add font atlas used for rendering characters
 *
 * @param atlas Font atlas
 *
 * @throws std::runtime_error If text renderer does not support font atlases
 */
void TextRenderer::addFontAtlas(const FontAtlas &atlas) {
    throw std::runtime_error("TextRenderer::addFontAtlas(): Selected text renderer does not support font atlases");
}

/**
 * @brief Set uniform buffer object used for rendering
 *
 * @param ubo Uniform buffer object
 */
void TextRenderer::setUniformBuffers(UniformBufferObject ubo) {
    this->_ubo = ubo;
}

/**
 * @brief Set viewport size
 *
 * @param width Viewport width
 * @param height Viewport height
 */
void TextRenderer::setViewportSize(unsigned int width, unsigned int height) {
    this->_viewportWidth = width;
    this->_viewportHeight = height;
}

/**
 * @brief Set glyph cache used for rendering
 *
 * @param cache Glyph cache
 */
void TextRenderer::setCache(std::shared_ptr<GlyphCache> cache) {
    this->_cache = cache;
}

}  // namespace vft
