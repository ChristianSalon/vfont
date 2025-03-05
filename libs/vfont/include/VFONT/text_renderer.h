/**
 * @file text_renderer.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>

#include "glyph_cache.h"
#include "tessellator.h"
#include "text_block.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @class TextRenderer
 *
 * @brief Creates vertex and index buffers for specified characters
 */
class TextRenderer {
protected:
    UniformBufferObject _ubo{glm::mat4{1.f}, glm::mat4{1.f}};
    unsigned int _viewportWidth{0};
    unsigned int _viewportHeight{0};

    std::vector<std::shared_ptr<TextBlock>> _textBlocks{}; /**< All text blocks to be rendered */
    std::shared_ptr<GlyphCache> _cache{nullptr};
    std::unique_ptr<Tessellator> _tessellator{nullptr};

public:
    TextRenderer() = default;
    virtual ~TextRenderer() = default;

    virtual void initialize();
    virtual void destroy();
    virtual void add(std::shared_ptr<TextBlock> text);
    virtual void draw() = 0;
    virtual void update() = 0;

    virtual void setUniformBuffers(UniformBufferObject ubo);
    virtual void setViewportSize(unsigned int width, unsigned int height);
    virtual void setCache(std::shared_ptr<GlyphCache> cache);
};

}  // namespace vft
