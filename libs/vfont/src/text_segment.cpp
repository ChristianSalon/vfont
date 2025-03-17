/**
 * @file text_segment.cpp
 * @author Christian Saloň
 */

#include "text_segment.h"

namespace vft {

/**
 * @brief TextSegment constructor
 *
 * @param font Font of characters in segment
 * @param fontSize Font size of characters in segment
 */
TextSegment::TextSegment(std::shared_ptr<Font> font, unsigned int fontSize) : _font{font}, _fontSize{fontSize} {}

/**
 * @brief Add unicode code points to segment at given position
 *
 * @param codePoints Unicode code points
 * @param start Index where to start adding code points
 */
void TextSegment::add(const std::vector<uint32_t> &codePoints, unsigned int start) {
    if (start == std::numeric_limits<unsigned int>::max()) {
        start = this->_codePoints.size();
    }

    if (start > this->_codePoints.size()) {
        throw std::out_of_range("TextSegment::add(): Start index is out of bounds");
    }

    // Add unicode code points to segment
    this->_codePoints.insert(this->_codePoints.begin() + start, codePoints.begin(), codePoints.end());

    // Shape segment and update characters
    this->_shape();
}

/**
 * @brief Remove unicode code points from segment
 *
 * @param start Index from which to start removing code points
 * @param count Number of code points to remove
 */
void TextSegment::remove(unsigned int start, unsigned int count) {
    if (count == 0) {
        return;
    }

    if (start >= this->_codePoints.size()) {
        throw std::out_of_range("TextSegment::remove(): Start index is out of bounds");
    }

    if (start + count > this->_codePoints.size()) {
        throw std::out_of_range("TextSegment::remove(): Range exceeds available code points");
    }

    this->_codePoints.erase(this->_codePoints.begin() + start, this->_codePoints.begin() + start + count);

    // Shape segment and update characters
    this->_shape();
}

/**
 * @brief Set and apply transform of text block to characters in segment
 *
 * @param transform Transform matrix
 */
void TextSegment::setTransform(glm::mat4 transform) {
    this->_transform = transform;

    for (Character &character : this->_characters) {
        character.setTransform(this->_transform);
    }
}

/**
 * @brief Get transform of text block
 *
 * @return Transform matrix
 */
glm::mat4 TextSegment::getTransform() const {
    return this->_transform;
}

/**
 * @brief Get unicode code points in segment
 *
 * @return Code points
 */
const std::vector<uint32_t> &TextSegment::getCodePoints() {
    return this->_codePoints;
}

/**
 * @brief Get characters in segment
 *
 * @return Characters
 */
std::vector<Character> &TextSegment::getCharacters() {
    return this->_characters;
}

/**
 * @brief Get number of unicode code points in segment
 *
 * @return Number of code points
 */
unsigned int TextSegment::getCodePointCount() const {
    return this->_codePoints.size();
}

/**
 * @brief Get number of characters in segment
 *
 * @return Number of charcters
 */
unsigned int TextSegment::getCharacterCount() const {
    return this->_characters.size();
}

/**
 * @brief Get font of characters in segment
 *
 * @return Font
 */
std::shared_ptr<Font> TextSegment::getFont() const {
    return this->_font;
}

/**
 * @brief Get font size of characters in segment
 *
 * @return Font size
 */
unsigned int TextSegment::getFontSize() const {
    return this->_fontSize;
}

/**
 * @brief Shape all code points in segment
 */
void TextSegment::_shape() {
    // Shape whole segment
    std::vector<std::vector<ShapedCharacter>> shaped = Shaper::shape(this->_codePoints, this->_font, this->_fontSize);

    unsigned int originalCharacterCount = this->_characters.size();

    // Create characters from output of shaping and append to character vector
    unsigned int shapedLineCount = 0;
    for (const std::vector<ShapedCharacter> &shapedLine : shaped) {
        for (const ShapedCharacter &shapedCharacter : shapedLine) {
            Character character{shapedCharacter.glyphId, 0, this->_font, this->_fontSize};
            character.setAdvance(glm::vec2(shapedCharacter.xAdvance, shapedCharacter.yAdvance));
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }

        // On last iteration do not add new line
        shapedLineCount++;
        if (shapedLineCount != shaped.size()) {
            // Add new line
            Character character{0, vft::U_LF, this->_font, this->_fontSize};
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }
    }

    // Delete unwanted characters from the start of character vector
    // By implementing it this way, glyphs are preserved in cache
    this->_characters.erase(this->_characters.begin(), this->_characters.begin() + originalCharacterCount);
}

}  // namespace vft
