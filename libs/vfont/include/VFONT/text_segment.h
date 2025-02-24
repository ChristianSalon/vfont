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
#include "tessellator.h"

namespace vft {

/**
 * @class TextSegment
 *
 * @brief Groups together characters which are grouped together in a text block and have the same font and size
 */
class TextSegment {
protected:
    std::shared_ptr<Font> _font;  /// Font used by characters in segment
    unsigned int _fontSize;       /// Font size used in segment

    glm::mat4 _transform;

    std::vector<uint32_t> _codePoints;   /// Unicode codepoints to render
    std::vector<Character> _characters;  /// Characters to render

public:
    TextSegment(std::shared_ptr<Font> font, unsigned int fontSize);

    void add(const std::vector<uint32_t> &codePoints,
             std::shared_ptr<Tessellator> tessellator,
             unsigned int start = std::numeric_limits<unsigned int>::max());
    void remove(unsigned int start, std::shared_ptr<Tessellator> tessellator, unsigned int count = 1);

    void setTransform(glm::mat4 transform);
    glm::mat4 getTransform() const;

    const std::vector<uint32_t> &getCodePoints();
    std::vector<Character> &getCharacters();
    unsigned int getCodePointCount() const;
    unsigned int getCharacterCount() const;

    std::shared_ptr<Font> getFont() const;
    unsigned int getFontSize() const;

protected:
    void _shape(std::shared_ptr<Tessellator> tessellator);
};

}  // namespace vft
