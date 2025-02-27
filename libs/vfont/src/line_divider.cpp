/**
 * @file line_divider.cpp
 * @author Christian Saloň
 */

#include "line_divider.h"

namespace vft {

const std::map<unsigned int, LineData> &LineDivider::divide(unsigned int startCharacterIndex) {
    if (startCharacterIndex >= this->_characters.size()) {
        throw std::out_of_range("LineDivider::divide(): Start index is out of bounds");
    }

    if (this->_characters.size() == 0) {
        this->_lines = {};
        return this->_lines;
    }

    // Get line for first character
    auto lineIterator = this->_lines.upper_bound(startCharacterIndex);
    if (!this->_lines.empty()) {
        if (lineIterator != this->_lines.begin()) {
            // Get line at which is character at position start - 1
            lineIterator = std::prev(lineIterator);
            // Erase all lines after line at which is character at position start - 1
            this->_lines.erase(std::next(lineIterator), this->_lines.end());
        }
    } else {
        // If there are no lines create one
        this->_lines.insert({0, LineData{0, 0, 0, 0}});
        lineIterator = this->_lines.begin();
    }

    // Restore pen position
    glm::vec2 pen{0, lineIterator->second.y};
    if (startCharacterIndex != 0) {
        const Character &previousCharacter = this->_characters.at(startCharacterIndex);
        glm::vec2 scale = previousCharacter.getFont()->getScalingVector(previousCharacter.getFontSize());
        pen = previousCharacter.getPosition() + previousCharacter.getAdvance();
    }

    for (unsigned int characterIndex = startCharacterIndex; characterIndex < this->_characters.size();
         characterIndex++) {
        Character &character = this->_characters[characterIndex];

        if ((this->_maxLineSize > 0 && pen.x + character.glyph.getWidth() > this->_maxLineSize) ||
            character.getCodePoint() == vft::U_LF) {
            // Set pen position to start of new line
            pen.x = 0;
            pen.y += character.getFontSize();

            // Character should be on new line
            this->_lines.insert({characterIndex, LineData{character.getAdvance().x,
                                                          static_cast<double>(character.getFontSize()), 0, pen.y}});

            continue;
        }

        // Update pen position
        pen += character.getAdvance();
        // Update width of line
        this->_lines.rbegin()->second.width += character.getAdvance().x;

        // Check if font size of current character is bigger than the height of the line on which the current
        // character is
        if (character.getFontSize() > this->_lines.rbegin()->second.height) {
            // Update height and y coordinate of line
            this->_lines.rbegin()->second.y += character.getFontSize() - this->_lines.rbegin()->second.height;
            this->_lines.rbegin()->second.height = character.getFontSize();

            // Update y coordinate of pen
            pen.y += character.getFontSize() - pen.y;
        }
    }

    return this->_lines;
}

void LineDivider::setCharacters(const std::vector<Character> &characters) {
    this->_characters = characters;
}

void LineDivider::setMaxLineSize(double maxLineSize) {
    this->_maxLineSize = maxLineSize;
}

std::pair<unsigned int, LineData> LineDivider::getLineOfCharacter(unsigned int characterIndex) {
    if (this->_lines.empty()) {
        throw std::out_of_range("LineDivider::getLineOfCharacter(): Character index is out of bounds");
    }

    auto lineIterator = this->_lines.upper_bound(characterIndex);
    if (lineIterator != this->_lines.begin()) {
        return *std::prev(lineIterator);
    }

    throw std::runtime_error("LineDivider::getLineOfCharacter(): Such line does not exist");
}

}  // namespace vft
