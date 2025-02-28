/**
 * @file text_block.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "character.h"
#include "font.h"
#include "line_divider.h"
#include "shaper.h"
#include "tessellator.h"
#include "text_segment.h"
#include "text_align_strategy.h"

namespace vft {

/**
 * @class TextBlock
 *
 * @brief Groups together characters which are rendered
 */
class TextBlock {
public:
    std::function<void()> onTextChange; /**< Callback when text in block changes */

protected:
    std::shared_ptr<Font> _font; /**< Font of text in text block */

    bool _kerning;          /**< Indicates whether to use kerning in text block */
    bool _wrapping;         /**< Indicates whether to use wrapping in text block */
    unsigned int _fontSize; /**< Font size in text block */
    int _width;             /**< Width of text block. -1 indicates unlimited width */

    int _penX; /**< X coordinate of current pen position */
    int _penY; /**< Y coordinate of current pen position */

    glm::mat4 _transform; /**< Transform matrix of text block */
    glm::vec4 _color;     /**< Color of text */
    glm::vec3 _position;  /**< Position of text block */

    std::unique_ptr<TextAlignStrategy> _textAlign;

    std::list<TextSegment> _segments; /**< Text segments which include characters to render */
    LineDivider _lineDivider;

    std::shared_ptr<Tessellator> _tessellator;

public:
    TextBlock(std::shared_ptr<Font> font,
              unsigned int fontSize,
              glm::vec4 color,
              glm::vec3 position,
              int width = -1,
              bool kerning = true,
              bool wrapping = false);

    void scale(float x, float y, float z);
    void translate(float x, float y, float z);
    void rotate(float x, float y, float z);

    void add(std::vector<uint32_t> codePoints, unsigned int start = std::numeric_limits<unsigned int>::max());
    void add(std::string text, unsigned int start = std::numeric_limits<unsigned int>::max());
    void remove(unsigned int start = std::numeric_limits<unsigned int>::max(), unsigned int count = 1);
    void clear();

    void setFont(std::shared_ptr<Font> font);
    void setFontSize(unsigned int fontSize);
    void setColor(glm::vec4 color);

    void setPosition(glm::vec3 position);
    void setTransform(glm::mat4 transform);
    void setWidth(int width);
    void setKerning(bool kerning);
    void setWrapping(bool wrapping);
    void setTessellationStrategy(std::shared_ptr<Tessellator> tessellator);
    void setTextAlign(std::unique_ptr<TextAlignStrategy> textAlign);

    std::vector<Character> getCharacters();
    unsigned int getCharacterCount();
    std::vector<uint32_t> getCodePoints();
    unsigned int getCodePointCount();

    std::shared_ptr<Font> getFont() const;
    glm::vec4 getColor() const;
    glm::vec3 getPosition() const;
    glm::mat4 getTransform() const;
    int getWidth() const;
    bool getKerning() const;
    bool getWrapping() const;
    unsigned int getFontSize() const;

protected:
    void _updateCharacters();
    void _updateTransform();

    void _updateCharacterPositions(unsigned int start);
    std::list<TextSegment>::iterator _mergeSegmentsIfPossible(std::list<TextSegment>::iterator first,
                                                              std::list<TextSegment>::iterator second);

    TextSegment &_getSegmentBasedOnCodePointGlobalIndex(unsigned int index);
    std::list<TextSegment>::iterator _getSegmentIteratorBasedOnCodePointGlobalIndex(unsigned int index);
    TextSegment &_getSegmentBasedOnCharacterGlobalIndex(unsigned int index);
    std::list<TextSegment>::iterator _getSegmentIteratorBasedOnCharacterGlobalIndex(unsigned int index);
    Character &_getCharacterBasedOnCharacterGlobalIndex(unsigned int index);
    std::vector<Character>::iterator _getCharacterIteratorBasedOnCharacterGlobalIndex(unsigned int index);

    unsigned int _getCodePointGlobalIndexBasedOnSegment(const TextSegment &segment);
    unsigned int _getCharacterGlobalIndexBasedOnSegment(const TextSegment &segment);
    unsigned int _getCharacterGlobalIndexBasedOnCharacter(const Character &character);
};

}  // namespace vft
