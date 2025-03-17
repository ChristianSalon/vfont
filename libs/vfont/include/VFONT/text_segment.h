/**
 * @file text_segment.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <hb.h>
#include <glm/mat4x4.hpp>

#include "character.h"
#include "font.h"
#include "shaper.h"

namespace vft {

/**
 * @brief Groups together characters which have same properties (font and font size)
 */
class TextSegment {
protected:
    std::shared_ptr<Font> _font{nullptr}; /**< Font used by characters in segment */
    unsigned int _fontSize{0};            /**< Font size used in segment */

    glm::mat4 _transform{1.f}; /**< Transform matrix of text block */

    std::vector<uint32_t> _codePoints{};  /**< Unicode code points to render */
    std::vector<Character> _characters{}; /**< Characters to render */

public:
    TextSegment(std::shared_ptr<Font> font, unsigned int fontSize);

    void add(const std::vector<uint32_t> &codePoints, unsigned int start = std::numeric_limits<unsigned int>::max());
    void remove(unsigned int start, unsigned int count = 1);

    void setTransform(glm::mat4 transform);
    glm::mat4 getTransform() const;

    const std::vector<uint32_t> &getCodePoints();
    std::vector<Character> &getCharacters();
    unsigned int getCodePointCount() const;
    unsigned int getCharacterCount() const;

    std::shared_ptr<Font> getFont() const;
    unsigned int getFontSize() const;

protected:
    void _shape();
};

}  // namespace vft
