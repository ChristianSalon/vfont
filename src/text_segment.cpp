﻿/**
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
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of text in segment
 * @param language Language of text in segment
 */
TextSegment::TextSegment(std::shared_ptr<Font> font,
                         unsigned int fontSize,
                         hb_direction_t direction,
                         hb_script_t script,
                         hb_language_t language)
    : _font{font}, _fontSize{fontSize}, _direction{direction}, _script{script}, _language{language} {}

/**
 * @brief Add utf-32 text to segment at given position
 *
 * @param text Utf-32 encoded text
 * @param start Index where to start adding code points
 */
void TextSegment::add(const std::u32string &text, unsigned int start) {
    if (start == std::numeric_limits<unsigned int>::max()) {
        start = this->_text.size();
    }

    if (start > this->_text.size()) {
        throw std::out_of_range("TextSegment::add(): Start index is out of bounds");
    }

    // Add unicode code points to segment
    this->_text.insert(this->_text.begin() + start, text.begin(), text.end());

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

    if (start >= this->_text.size()) {
        throw std::out_of_range("TextSegment::remove(): Start index is out of bounds");
    }

    if (start + count > this->_text.size()) {
        throw std::out_of_range("TextSegment::remove(): Range exceeds available code points");
    }

    this->_text.erase(this->_text.begin() + start, this->_text.begin() + start + count);

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
 * @brief Get utf-32 encoded text in segment
 *
 * @return Utf-32 encoded text
 */
const std::u32string &TextSegment::getText() {
    return this->_text;
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
    return this->_text.size();
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
 * @brief Getter for direction of text in segment (e.g., left-to-right, right-to-left)
 *
 * @return Direction of text
 */
hb_direction_t TextSegment::getDirection() const {
    return this->_direction;
}

/**
 * @brief Getter for script of text in segment
 *
 * @return Script of text
 */
hb_script_t TextSegment::getScript() const {
    return this->_script;
}

/**
 * @brief Getter for language of text in segment
 *
 * @return Language of text
 */
hb_language_t TextSegment::getLanguage() const {
    return this->_language;
}

/**
 * @brief Shape all code points in segment
 */
void TextSegment::_shape() {
    // Shape whole segment
    std::vector<std::vector<ShapedCharacter>> shaped =
        Shaper::shape(this->_text, this->_font, this->_fontSize, this->_direction, this->_script, this->_language);

    unsigned int originalCharacterCount = this->_characters.size();

    // Create characters from output of shaping and append to character vector
    unsigned int shapedLineCount = 0;
    for (const std::vector<ShapedCharacter> &shapedLine : shaped) {
        for (const ShapedCharacter &shapedCharacter : shapedLine) {
            Character character{shapedCharacter.glyphId, 0, this->_font, this->_fontSize};
            character.setAdvance(glm::vec2{shapedCharacter.xAdvance, shapedCharacter.yAdvance});
            character.setOffset(glm::vec2{shapedCharacter.xOffset, shapedCharacter.yOffset});
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }

        // On last iteration do not add new line
        shapedLineCount++;
        if (shapedLineCount != shaped.size()) {
            // Add new line
            Character character{0, U_LF, this->_font, this->_fontSize};
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }
    }

    // Delete unwanted characters from the start of character vector
    // By implementing it this way, glyphs are preserved in cache
    this->_characters.erase(this->_characters.begin(), this->_characters.begin() + originalCharacterCount);
}

}  // namespace vft
