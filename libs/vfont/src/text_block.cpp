/**
 * @file text_block.cpp
 * @author Christian Saloň
 */

#include "text_block.h"

namespace vft {

/**
 * @brief TextBloc constructor
 *
 * @param font Font used in text block
 * @param fontSize Font size in pixels
 * @param color Font color
 * @param position Position of text block in scene
 * @param width Width of text block. -1 indicates unlimited width
 * @param kerning Indicates whether to use kerning in block
 * @param wrapping Indicates whether to use wrapping in block
 */
TextBlock::TextBlock(std::shared_ptr<Font> font,
                     unsigned int fontSize,
                     glm::vec4 color,
                     glm::vec3 position,
                     int width,
                     bool kerning,
                     bool wrapping)
    : _font{font},
      _fontSize{fontSize},
      _penX{0},
      _penY{static_cast<int>(fontSize)},
      _transform{glm::mat4(1.f)},
      _position{glm::vec3(0, 0, 0)} {
    this->setKerning(kerning);
    this->setWrapping(wrapping);
    this->setWidth(width);
    this->setColor(color);
    this->setPosition(position);
}

void TextBlock::add(std::vector<uint32_t> codePoints, unsigned int start) {
    if (codePoints.size() == 0) {
        return;
    }

    // Check if parameter start was set
    if (start == std::numeric_limits<unsigned int>::max()) {
        start = this->getCodePointCount();
    }

    // Check if code point at index start exists or if start is one index after last code point
    if (start > this->getCodePointCount()) {
        throw std::out_of_range("add(): Start index is out of bounds");
    }

    // Global index of first character where to start updating line data and position
    unsigned int newSegmentCharacterGlobalIndex = 0;

    // Edit text segments
    if (this->_segments.size() == 0) {
        // No text segment exists, create one
        TextSegment newSegment{this->_font, this->_fontSize};
        newSegment.setTransform(this->_transform);
        newSegment.add(codePoints, this->_tessellator);

        this->_segments.push_back(newSegment);
    } else {
        // Segment in which we want to add text
        TextSegment &segment = this->_getSegmentBasedOnCodePointGlobalIndex(start);
        auto segmentIterator = this->_getSegmentIteratorBasedOnCodePointGlobalIndex(start);

        if (this->_font == segment.getFont() && this->_fontSize == segment.getFontSize()) {
            // Add text to selected segment at specified position
            // No need to create a new text segment
            segment.add(codePoints, this->_tessellator, start - this->_getCodePointGlobalIndexBasedOnSegment(segment));
        } else {
            TextSegment newSegment{this->_font, this->_fontSize};
            newSegment.setTransform(this->_transform);
            newSegment.add(codePoints, this->_tessellator);
            unsigned int localIndex = start - this->_getCodePointGlobalIndexBasedOnSegment(segment);

            if (localIndex == 0) {
                // Add new text before text segment
                this->_segments.insert(segmentIterator, newSegment);
            } else if (localIndex == segment.getCodePointCount()) {
                // Add new text after text segment
                this->_segments.insert(std::next(segmentIterator), newSegment);
            } else {
                // Split text segment and insert new text in between

                // Create right segment and add code points from range <localIndex, end)
                TextSegment rightSegment{segment.getFont(), segment.getFontSize()};
                rightSegment.setTransform(this->_transform);
                rightSegment.add(std::vector<uint32_t>(std::next(segment.getCodePoints().begin(), localIndex),
                                                       segment.getCodePoints().end()),
                                 this->_tessellator);

                // Erase code points from left segment from range <localIndex, end)
                segment.remove(localIndex, this->_tessellator);

                // Add new middle segment
                this->_segments.insert(std::next(segmentIterator), newSegment);
                // Add right segment
                this->_segments.insert(std::next(segmentIterator, 2), rightSegment);
            }
        }

        newSegmentCharacterGlobalIndex = this->_getCharacterGlobalIndexBasedOnSegment(segment);
    }

    // Calculate new line data
    this->_lineDivider.setCharacters(this->getCharacters());
    this->_lineDivider.divide(newSegmentCharacterGlobalIndex);

    // Set character positions
    this->_updateCharacterPositions(newSegmentCharacterGlobalIndex);

    // onTextChange callback
    if (this->onTextChange) {
        this->onTextChange();
    }
}

void TextBlock::add(std::string text, unsigned int start) {
    std::vector<uint32_t> codePoints;
    for (unsigned char character : text) {
        codePoints.push_back(character);
    }

    this->add(codePoints, start);
}

/**
 * @brief Remove characters from end of text block
 *
 * @param count Amount of characters to remove
 */
void TextBlock::remove(unsigned int start, unsigned int count) {
    if (count == 0 || this->getCodePointCount() == 0) {
        return;
    }

    // Check if parameter start was set
    if (start == std::numeric_limits<unsigned int>::max()) {
        start = this->getCodePointCount() - 1;
    }

    if (start >= this->getCodePointCount()) {
        throw std::out_of_range("remove(): Start index is out of bounds");
    }

    if (start + count > this->getCodePointCount()) {
        throw std::out_of_range("remove(): Range exceeds available characters");
    }

    // Edit text segments
    unsigned int globalIndex = 0;
    auto segmentIterator = this->_segments.begin();

    while (segmentIterator != this->_segments.end() && count > 0) {
        unsigned int segmentCodePointCount = segmentIterator->getCodePointCount();
        if (start < globalIndex + segmentCodePointCount) {
            unsigned int localStart = start - globalIndex;

            if (localStart == 0 && count >= segmentCodePointCount) {
                // Remove entire segment
                segmentIterator = this->_segments.erase(segmentIterator);
                count -= segmentCodePointCount;
                globalIndex += segmentCodePointCount;

                continue;
            } else if (localStart == 0) {
                // Remove code points from start to non-end of segment
                segmentIterator->remove(0, this->_tessellator, count);
                count = 0;
            } else if (count >= segmentCodePointCount - localStart) {
                // Remove from non-start to end of segment
                unsigned int charactersToRemove = segmentCodePointCount - localStart;
                segmentIterator->remove(localStart, this->_tessellator, charactersToRemove);
                count -= charactersToRemove;
            } else {
                // Remove from non-start to non-end of segment
                segmentIterator->remove(localStart, this->_tessellator, count);
                count = 0;
            }
        }

        globalIndex += segmentCodePointCount;
        segmentIterator = std::next(segmentIterator);
    }

    if (!this->getCharacterCount() == 0) {
        // Global index of first character where to start updating line data and position
        unsigned int segmentCharacterGlobalIndex = 0;

        // Merge segments if necessary
        if (std::distance(this->_segments.begin(), segmentIterator) >= 2) {
            auto left = std::prev(segmentIterator, 2);
            auto middle = std::prev(segmentIterator, 1);

            segmentCharacterGlobalIndex = this->_getCharacterGlobalIndexBasedOnSegment(*left);

            auto right = this->_mergeSegmentsIfPossible(left, middle);

            if (right != this->_segments.begin() && right != this->_segments.end()) {
                right = this->_mergeSegmentsIfPossible(std::prev(right), right);
            }
        }

        // Calculate new line data
        this->_lineDivider.setCharacters(this->getCharacters());
        this->_lineDivider.divide(segmentCharacterGlobalIndex);

        // Set character positions
        this->_updateCharacterPositions(segmentCharacterGlobalIndex);
    }

    if (this->onTextChange) {
        this->onTextChange();
    }
}

/**
 * @brief Remove all characters in text block
 */
void TextBlock::clear() {
    this->_segments.clear();
}

/**
 * @brief Apply scale to text block
 *
 * @param x Scale in X direction
 * @param y Scale in Y direction
 * @param z Scale in Z direction
 */
void TextBlock::scale(float x, float y, float z) {
    this->setTransform(glm::scale(this->getTransform(), glm::vec3(x, y, z)));
}

/**
 * @brief Apply translation to text block
 *
 * @param x Translation in X direction
 * @param y Translation in Y direction
 * @param z Translation in Z direction
 */
void TextBlock::translate(float x, float y, float z) {
    this->setTransform(glm::translate(this->getTransform(), glm::vec3(x, y, z)));
}

/**
 * @brief Apply rotation to text block
 *
 * @param x Rotation around the X axis
 * @param y Rotation around the Y axis
 * @param z Rotation around the Z axis
 */
void TextBlock::rotate(float x, float y, float z) {
    glm::mat4 newMatrix = this->getTransform();
    newMatrix = glm::rotate(newMatrix, glm::radians(x), glm::vec3(1.f, 0.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(y), glm::vec3(0.f, 1.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(z), glm::vec3(0.f, 0.f, 1.f));

    this->setTransform(newMatrix);
}

/**
 * @brief Set font in text block
 *
 * @param font Font
 */
void TextBlock::setFont(std::shared_ptr<Font> font) {
    this->_font = font;
}

/**
 * @brief Set font size of characters in text block
 *
 * @param fontSize Font size in pixels
 */
void TextBlock::setFontSize(unsigned int fontSize) {
    this->_fontSize = fontSize;
}

/**
 * @brief Set color of characters in text block
 *
 * @param color Color
 */
void TextBlock::setColor(glm::vec4 color) {
    this->_color = color;
}

/**
 * @brief Set position of text block in scene
 *
 * @param position 3D position
 */
void TextBlock::setPosition(glm::vec3 position) {
    glm::mat4 transform = glm::translate(this->getTransform(), -this->_position);
    this->_position = position;
    this->setTransform(glm::translate(transform, this->_position));
}

/**
 * @brief Set transform of text block and update characters
 *
 * @param transform Transform matrix
 */
void TextBlock::setTransform(glm::mat4 transform) {
    this->_transform = transform;
    this->_updateTransform();
}

/**
 * @brief Set width of text block. -1 indicates unlimited width
 *
 * @param width Width of text block
 */
void TextBlock::setWidth(int width) {
    this->_width = width;
    this->_lineDivider.setMaxLineSize(this->_width);
    this->_updateCharacters();
}

/**
 * @brief Check if font supports kerning, apply if supported and update characters
 *
 * @param kerning Indicates whether to use kerning in text block
 */
void TextBlock::setKerning(bool kerning) {
    if (kerning && !this->_font->supportsKerning()) {
        std::cerr << "Font " << this->_font->getFontFamily() << " does not support kerning" << std::endl;
        return;
    }

    if (this->getKerning() != kerning) {
        this->_kerning = kerning;
        this->_updateCharacters();
    }
}

/**
 * @brief Set whether to use wrapping in text block
 *
 * @param kerning Indicates whether to use wrapping in text block
 */
void TextBlock::setWrapping(bool wrapping) {
    if (this->getWrapping() != wrapping) {
        this->_wrapping = wrapping;
        this->_updateCharacters();
    }
}

void TextBlock::setTessellationStrategy(std::shared_ptr<Tessellator> tessellator) {
    this->_tessellator = tessellator;
}

/**
 * @brief Get all characters in text block
 *
 * @return Characters in text block
 */
std::vector<Character> TextBlock::getCharacters() {
    std::vector<Character> characters;
    for (TextSegment &segment : this->_segments) {
        characters.insert(characters.end(), segment.getCharacters().begin(), segment.getCharacters().end());
    }

    return characters;
}

unsigned int TextBlock::getCharacterCount() {
    unsigned int count = 0;
    for (const TextSegment &segment : this->_segments) {
        count += segment.getCharacterCount();
    }

    return count;
}

std::vector<uint32_t> TextBlock::getCodePoints() {
    std::vector<uint32_t> codePoints;
    for (TextSegment &segment : this->_segments) {
        codePoints.insert(codePoints.end(), segment.getCodePoints().begin(), segment.getCodePoints().end());
    }

    return codePoints;
}

unsigned int TextBlock::getCodePointCount() {
    unsigned int count = 0;
    for (const TextSegment &segment : this->_segments) {
        count += segment.getCodePointCount();
    }

    return count;
}

/**
 * @brief Get font used in text block
 *
 * @return Font used in text block
 */
std::shared_ptr<Font> TextBlock::getFont() const {
    return this->_font;
}

/**
 * @brief Get font size used in text block
 *
 * @return Font size used in text block
 */
unsigned int TextBlock::getFontSize() const {
    return this->_fontSize;
}

/**
 * @brief Get font color used in text block
 *
 * @return Font color used in text block
 */
glm::vec4 TextBlock::getColor() const {
    return this->_color;
}

/**
 * @brief Get position of text block
 *
 * @return Position of text block
 */
glm::vec3 TextBlock::getPosition() const {
    return this->_position;
}

/**
 * @brief Get width of text block
 *
 * @return Width of text block
 */
int TextBlock::getWidth() const {
    return this->_width;
}

/**
 * @brief Get transform of text block
 *
 * @return Transform matrix
 */
glm::mat4 TextBlock::getTransform() const {
    return this->_transform;
}

/**
 * @brief Get whether kerning is used in text block
 *
 * @return True if kerning is used, else false
 */
bool TextBlock::getKerning() const {
    return this->_kerning;
}

/**
 * @brief Get whether wrapping is used in text block
 *
 * @return True if wrapping is used, else false
 */
bool TextBlock::getWrapping() const {
    return this->_wrapping;
}

/**
 * @brief Update all characters and use new properties of text block
 */
void TextBlock::_updateCharacters() {
    unsigned int codePointCount = this->getCodePointCount();

    for (TextSegment &segment : this->_segments) {
        this->_font = segment.getFont();
        this->_fontSize = segment.getFontSize();

        this->add(segment.getCodePoints());
    }

    this->remove(0, codePointCount);
}

/**
 * @brief Update all characters using the new transform of text block
 */
void TextBlock::_updateTransform() {
    for (TextSegment &segment : this->_segments) {
        for (Character &character : segment.getCharacters()) {
            character.setTransform(this->getTransform());
        }
    }
}

void TextBlock::_updateCharacterPositions(unsigned int start) {
    // Index of first character that needs recalculating position
    // Index of first character on line
    unsigned int globalCharacterIndex = this->_lineDivider.getLineOfCharacter(start).first;

    // Restore pen position
    int penX = 0;
    int penY = 0;

    // Apply calculated positions by shaper and LineData to characters
    while (globalCharacterIndex < this->getCharacterCount()) {
        auto line = this->_lineDivider.getLineOfCharacter(globalCharacterIndex);

        if (line.first == globalCharacterIndex) {
            // Character is first on current line, restore pen position
            penX = 0;
            penY = line.second.y;
        }

        // Set character position
        Character &character = this->_getCharacterBasedOnCharacterGlobalIndex(globalCharacterIndex);
        character.setPosition(glm::vec2(penX, penY));

        // Update pen position
        penX += character.getAdvance().x;
        penY += character.getAdvance().y;

        globalCharacterIndex++;
    }
}

std::list<TextSegment>::iterator TextBlock::_mergeSegmentsIfPossible(std::list<TextSegment>::iterator first,
                                                                     std::list<TextSegment>::iterator second) {
    if (first == this->_segments.end() || second == this->_segments.end()) {
        throw std::out_of_range("_mergeSegmentsIfPossible(): Invalid segment iterator");
    }

    if (first->getFont() == second->getFont() && first->getFontSize() == second->getFontSize()) {
        // Move characters from second segment to first
        first->add(second->getCodePoints(), this->_tessellator);

        // Return iterator to segment one after merged segment
        return this->_segments.erase(second);
    }

    // Return iterator to segment one after second segment
    return std::next(second);
}

TextSegment &TextBlock::_getSegmentBasedOnCodePointGlobalIndex(unsigned int index) {
    if (index > this->getCodePointCount()) {
        throw std::out_of_range("_getSegmentBasedOnCodePointGlobalIndex(): Range exceeds available characters");
    }

    if (!this->_segments.empty() && index == this->getCodePointCount()) {
        return this->_segments.back();
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCodePointCount()) {
            return segment;
        }

        currentIndex += segment.getCodePointCount();
    }

    throw std::runtime_error("_getSegmentBasedOnCodePointGlobalIndex(): Such segment does not exist");
}

std::list<TextSegment>::iterator TextBlock::_getSegmentIteratorBasedOnCodePointGlobalIndex(unsigned int index) {
    if (index > this->getCodePointCount()) {
        throw std::out_of_range("_getSegmentIteratorBasedOnCodePointGlobalIndex(): Range exceeds available characters");
    }

    if (!this->_segments.empty() && index == this->getCodePointCount()) {
        return std::prev(this->_segments.end());
    }

    unsigned int currentIndex = 0;
    for (auto it = this->_segments.begin(); it != this->_segments.end(); it++) {
        if (index < currentIndex + it->getCodePointCount()) {
            return it;
        }

        currentIndex += it->getCodePointCount();
    }

    throw std::runtime_error("_getSegmentIteratorBasedOnCodePointGlobalIndex(): Such segment does not exist");
}

TextSegment &TextBlock::_getSegmentBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index > this->getCharacterCount()) {
        throw std::out_of_range("_getSegmentBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    if (!this->_segments.empty() && index == this->getCharacterCount()) {
        return this->_segments.back();
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCharacterCount()) {
            return segment;
        }

        currentIndex += segment.getCharacterCount();
    }

    throw std::runtime_error("_getSegmentBasedOnCharacterGlobalIndex(): Such segment does not exist");
}

std::list<TextSegment>::iterator TextBlock::_getSegmentIteratorBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index > this->getCharacterCount()) {
        throw std::out_of_range("_getSegmentIteratorBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    if (!this->_segments.empty() && index == this->getCharacterCount()) {
        return std::prev(this->_segments.end());
    }

    unsigned int currentIndex = 0;
    for (auto it = this->_segments.begin(); it != this->_segments.end(); it++) {
        if (index < currentIndex + it->getCharacterCount()) {
            return it;
        }

        currentIndex += it->getCharacterCount();
    }

    throw std::runtime_error("_getSegmentIteratorBasedOnCharacterGlobalIndex(): Such segment does not exist");
}

Character &TextBlock::_getCharacterBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index >= this->getCharacterCount()) {
        throw std::out_of_range("_getCharacterBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCharacterCount()) {
            return segment.getCharacters().at(index - currentIndex);
        }

        currentIndex += segment.getCharacterCount();
    }

    throw std::runtime_error("_getCharacterBasedOnCharacterGlobalIndex(): Such character does not exist");
}

std::vector<Character>::iterator TextBlock::_getCharacterIteratorBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index >= this->getCharacterCount()) {
        throw std::out_of_range(
            "_getCharacterIteratorBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCharacterCount()) {
            return std::next(segment.getCharacters().begin(), index - currentIndex);
        }

        currentIndex += segment.getCharacterCount();
    }

    throw std::runtime_error("_getCharacterIteratorBasedOnCharacterGlobalIndex(): Such character does not exist");
}

unsigned int TextBlock::_getCodePointGlobalIndexBasedOnSegment(const TextSegment &segment) {
    unsigned int currentIndex = 0;
    for (const TextSegment &s : this->_segments) {
        if (&s == &segment) {
            return currentIndex;
        }

        currentIndex += s.getCodePointCount();
    }

    throw std::runtime_error("_getCodePointGlobalIndexBasedOnSegment(): Such segment does not exist");
}

unsigned int TextBlock::_getCharacterGlobalIndexBasedOnSegment(const TextSegment &segment) {
    unsigned int currentIndex = 0;
    for (const TextSegment &s : this->_segments) {
        if (&s == &segment) {
            return currentIndex;
        }

        currentIndex += s.getCharacterCount();
    }

    throw std::runtime_error("_getCharacterGlobalIndexBasedOnSegment(): Such segment does not exist");
}

unsigned int TextBlock::_getCharacterGlobalIndexBasedOnCharacter(const Character &character) {
    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        for (const Character &c : segment.getCharacters()) {
            if (&c == &character) {
                return currentIndex;
            }

            currentIndex++;
        }
    }

    throw std::runtime_error("_getCharacterGlobalIndexBasedOnCharacter(): Such character does not exist");
}

}  // namespace vft
