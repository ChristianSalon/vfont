/**
 * @file line_divider.cpp
 * @author Christian Saloň
 */

#include "line_divider.h"

namespace vft {

/**
 * @brief Divides characters starting at given index into lines
 *
 * @param startCharacterIndex Index of starting character which to divide
 *
 * @return Divided lines
 */
const std::map<unsigned int, LineData> &LineDivider::divide(unsigned int startCharacterIndex) {
    if (startCharacterIndex >= this->_characters.size()) {
        throw std::out_of_range("LineDivider::divide(): Start index is out of bounds");
    }

    if (this->_characters.size() == 0) {
        this->_lines = {};
        return this->_lines;
    }

    unsigned int firstCharacterOnLineIndex = 0;
    if (!this->_lines.empty()) {
        // One line after the line of character at start index
        auto lineIterator = this->_lines.upper_bound(startCharacterIndex);
        if (lineIterator != this->_lines.begin()) {
            // Line of character at start index
            lineIterator = std::prev(lineIterator);

            firstCharacterOnLineIndex = lineIterator->first;

            // Erase all lines after and including the line at which is character at start index
            this->_lines.erase(lineIterator, this->_lines.end());
        }
    }

    // Process first character on the first line that needs updating
    // Inserting now ensures that at least one line exists, avoids invalid line iterators
    this->_lines.insert(
        {firstCharacterOnLineIndex,
         LineData{this->_characters[firstCharacterOnLineIndex].getAdvance().x,
                  static_cast<double>(this->_characters[firstCharacterOnLineIndex].getFontSize()), 0,
                  this->_lines.empty()
                      ? static_cast<double>(this->_characters[firstCharacterOnLineIndex].getFontSize())
                      : this->_lines.rbegin()->second.y +
                            static_cast<double>(this->_characters[firstCharacterOnLineIndex].getFontSize())}});

    // Restore pen position with respect to newly added line
    glm::vec2 pen{this->_lines.rbegin()->second.width, this->_lines.rbegin()->second.y};

    for (unsigned int characterIndex = firstCharacterOnLineIndex + 1; characterIndex < this->_characters.size();
         characterIndex++) {
        Character &character = this->_characters[characterIndex];

        if ((this->_maxLineSize > 0 && pen.x + character.getAdvance().x > this->_maxLineSize) ||
            character.getCodePoint() == U_LF) {
            // Set pen position after the first character on new line
            pen.x = character.getAdvance().x;
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
            pen.y = this->_lines.rbegin()->second.y;
        }
    }

    return this->_lines;
}

/**
 * @brief Set characters which will be divided into lines
 *
 * @param characters All characters
 */
void LineDivider::setCharacters(const std::vector<Character> &characters) {
    this->_characters = characters;
}

/**
 * @brief Set the maximum size of lines
 *
 * @param maxLineSize
 */
void LineDivider::setMaxLineSize(double maxLineSize) {
    this->_maxLineSize = maxLineSize;
}

/**
 * @brief Get line on which is character at given index
 *
 * @param characterIndex Index of character
 *
 * @return Line on which is character
 */
std::pair<unsigned int, LineData> LineDivider::getLineOfCharacter(unsigned int characterIndex) const {
    if (this->_lines.empty()) {
        throw std::out_of_range("LineDivider::getLineOfCharacter(): Character index is out of bounds");
    }

    auto lineIterator = this->_lines.upper_bound(characterIndex);
    if (lineIterator != this->_lines.begin()) {
        return *std::prev(lineIterator);
    }

    throw std::runtime_error("LineDivider::getLineOfCharacter(): Such line does not exist");
}

/**
 * @brief Get all divided lines
 *
 * @return Divided lines
 */
const std::map<unsigned int, LineData> &LineDivider::getLines() const {
    return this->_lines;
};

}  // namespace vft
