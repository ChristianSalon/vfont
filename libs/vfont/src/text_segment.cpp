/**
 * @file text_segment.cpp
 * @author Christian Saloň
 */

#include "text_segment.h"

namespace vft {

TextSegment::TextSegment(std::shared_ptr<Font> font, unsigned int fontSize)
    : _font{font}, _fontSize{fontSize}, _transform{1.f} {}

void TextSegment::add(const std::vector<uint32_t> &codePoints,
                      std::shared_ptr<Tessellator> tessellator,
                      unsigned int start) {
    if (start == std::numeric_limits<unsigned int>::max()) {
        start = this->_codePoints.size();
    }

    if (start > this->_codePoints.size()) {
        throw std::out_of_range("TextSegment::add(): Start index is out of bounds");
    }

    // Add unicode code points to segment
    this->_codePoints.insert(this->_codePoints.begin() + start, codePoints.begin(), codePoints.end());

    // Shape segment and update characters
    this->_shape(tessellator);
}

void TextSegment::remove(unsigned int start, std::shared_ptr<Tessellator> tessellator, unsigned int count) {
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
    this->_shape(tessellator);
}

void TextSegment::setTransform(glm::mat4 transform) {
    this->_transform = transform;

    for (Character &character : this->_characters) {
        character.setTransform(this->_transform);
    }
}

glm::mat4 TextSegment::getTransform() const {
    return this->_transform;
}

const std::vector<uint32_t> &TextSegment::getCodePoints() {
    return this->_codePoints;
}

std::vector<Character> &TextSegment::getCharacters() {
    return this->_characters;
}

unsigned int TextSegment::getCodePointCount() const {
    return this->_codePoints.size();
}

unsigned int TextSegment::getCharacterCount() const {
    return this->_characters.size();
}

std::shared_ptr<Font> TextSegment::getFont() const {
    return this->_font;
}

unsigned int TextSegment::getFontSize() const {
    return this->_fontSize;
}

void TextSegment::_shape(std::shared_ptr<Tessellator> tessellator) {
    // Shape whole segment
    std::vector<std::vector<ShapedCharacter>> shaped = Shaper::shape(this->_codePoints, this->_font, this->_fontSize);

    unsigned int originalCharacterCount = this->_characters.size();

    // Create characters from output of shaping and append to character vector
    unsigned int shapedLineCount = 0;
    for (const std::vector<ShapedCharacter> &shapedLine : shaped) {
        for (const ShapedCharacter &shapedCharacter : shapedLine) {
            // Creates glyph if not in cache
            Glyph glyph = tessellator->composeGlyph(shapedCharacter.glyphId, this->_font, this->_fontSize);
            Character character{glyph, shapedCharacter.glyphId, this->_font, this->_fontSize};
            character.setAdvance(glm::vec2(shapedCharacter.xAdvance, shapedCharacter.yAdvance));
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }

        // On last iteration do not add new line
        shapedLineCount++;
        if (shapedLineCount != shaped.size()) {
            // Add new line
            Character character{Glyph{}, vft::U_LF, this->_font, this->_fontSize};
            character.setAdvance(glm::vec2(0, 0));
            character.setTransform(this->_transform);

            this->_characters.push_back(character);
        }
    }

    // Delete unwanted characters from the start of character vector
    // By implementing it this way, glyphs are preserved in cache
    this->_characters.erase(this->_characters.begin(), this->_characters.begin() + originalCharacterCount);
}

}  // namespace vft
