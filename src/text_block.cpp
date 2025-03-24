/**
 * @file text_block.cpp
 * @author Christian Saloň
 */

#include "text_block.h"

namespace vft {

/**
 * @brief TextBlock constructor
 */
TextBlock::TextBlock() : _font{nullptr} {
    this->setFontSize(0);
    this->setWidth(-1);
    this->setLineSpacing(1);
    this->setColor(glm::vec4(1, 1, 1, 1));
    this->setPosition(glm::vec3(0, 0, 0));
    this->setTextAlign(std::make_unique<LeftTextAlign>());
}

/**
 * @brief Add utf-32 encoded text to text block at given position
 *
 * @param text Utf-32 encoded text
 * @param start Position at which to start inserting text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u32string text,
                    unsigned int start,
                    hb_direction_t direction,
                    hb_script_t script,
                    hb_language_t language) {
    if (text.size() == 0) {
        return;
    }

    // If font is not set, characters can not be processed
    if (this->_font == nullptr) {
        throw std::runtime_error("TextBlock::add(): Font must be set before adding text to text block");
    }

    // Check if code point at index start exists or if start is one index after last code point
    if (start > this->getCodePointCount()) {
        throw std::out_of_range("TextBlock::add(): Start index is out of bounds");
    }

    // Global index of first character where to start updating line data and position
    unsigned int newSegmentCharacterGlobalIndex = 0;

    // Edit text segments
    if (this->_segments.size() == 0) {
        // No text segment exists, create one
        TextSegment newSegment{this->_font, this->_fontSize, direction, script, language};
        newSegment.setTransform(this->_transform);
        newSegment.add(text);

        this->_segments.push_back(newSegment);
    } else {
        // Segment in which we want to add text
        TextSegment &segment = this->_getSegmentBasedOnCodePointGlobalIndex(start);
        auto segmentIterator = this->_getSegmentIteratorBasedOnCodePointGlobalIndex(start);

        if (this->_font == segment.getFont() && this->_fontSize == segment.getFontSize() &&
            direction == segment.getDirection() && script == segment.getScript() && language == segment.getLanguage()) {
            // Add text to selected segment at specified position
            // No need to create a new text segment
            segment.add(text, start - this->_getCodePointGlobalIndexBasedOnSegment(segment));
        } else {
            TextSegment newSegment{this->_font, this->_fontSize, direction, script, language};
            newSegment.setTransform(this->_transform);
            newSegment.add(text);
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
                rightSegment.add(
                    std::u32string(std::next(segment.getText().begin(), localIndex), segment.getText().end()));

                // Erase code points from left segment from range <localIndex, end)
                segment.remove(localIndex);

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

/**
 * @brief Add utf-8 encoded text into text block at given position
 *
 * @param text Utf-8 encoded text
 * @param start Position at which to start inserting text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u8string text,
                    unsigned int start,
                    hb_direction_t direction,
                    hb_script_t script,
                    hb_language_t language) {
    this->add(Unicode::utf8ToUtf32(text), start, direction, script, language);
}

/**
 * @brief Add utf-16 encoded text into text block at given position
 *
 * @param text Utf-16 encoded text
 * @param start Position at which to start inserting text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u16string text,
                    unsigned int start,
                    hb_direction_t direction,
                    hb_script_t script,
                    hb_language_t language) {
    this->add(Unicode::utf16ToUtf32(text), start, direction, script, language);
}

/**
 * @brief Add utf-8 encoded text at the back of text bleck
 *
 * @param text Utf-8 encoded text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u8string text, hb_direction_t direction, hb_script_t script, hb_language_t language) {
    this->add(Unicode::utf8ToUtf32(text), this->getCodePointCount(), direction, script, language);
}

/**
 * @brief Add utf-16 encoded text at the back of text bleck
 *
 * @param text Utf-16 encoded text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u16string text, hb_direction_t direction, hb_script_t script, hb_language_t language) {
    this->add(Unicode::utf16ToUtf32(text), this->getCodePointCount(), direction, script, language);
}

/**
 * @brief Add utf-32 encoded text at the back of text bleck
 *
 * @param text Utf-32 encoded text
 * @param direction Direction in which to render text (e.g., left-to-right, right-to-left)
 * @param script Script of input text
 * @param language Language of input text
 */
void TextBlock::add(std::u32string text, hb_direction_t direction, hb_script_t script, hb_language_t language) {
    this->add(text, this->getCodePointCount(), direction, script, language);
}

/**
 * @brief Remove characters from text block
 *
 * @param start Position at which to start removing text
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
        throw std::out_of_range("TextBlock::remove(): Start index is out of bounds");
    }

    if (start + count > this->getCodePointCount()) {
        throw std::out_of_range("TextBlock::remove(): Range exceeds available characters");
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
                segmentIterator->remove(0, count);
                count = 0;
            } else if (count >= segmentCodePointCount - localStart) {
                // Remove from non-start to end of segment
                unsigned int charactersToRemove = segmentCodePointCount - localStart;
                segmentIterator->remove(localStart, charactersToRemove);
                count -= charactersToRemove;
            } else {
                // Remove from non-start to non-end of segment
                segmentIterator->remove(localStart, count);
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
 * @brief Remove all characters from text block
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
 * @brief Set font to use from now on
 *
 * @param font Font to use
 */
void TextBlock::setFont(std::shared_ptr<Font> font) {
    if (font == nullptr) {
        throw std::invalid_argument("TextBlock::setFont(): Font must not be null");
    }

    this->_font = font;
}

/**
 * @brief Set font size to use from now on
 *
 * @param fontSize Font size in pixels
 */
void TextBlock::setFontSize(unsigned int fontSize) {
    this->_fontSize = fontSize;
}

/**
 * @brief Set line spacing in text block. Default line spacing is 1
 *
 * @param lineSpacing New line spacing
 */
void TextBlock::setLineSpacing(double lineSpacing) {
    this->_lineSpacing = lineSpacing;
    this->_lineDivider.setLineSpacing(this->_lineSpacing);
}

/**
 * @brief Set color of characters in text block
 *
 * @param color New color
 */
void TextBlock::setColor(glm::vec4 color) {
    this->_color = color;
}

/**
 * @brief Set position of text block
 *
 * @param position 3D position
 */
void TextBlock::setPosition(glm::vec3 position) {
    glm::mat4 transform = glm::translate(this->getTransform(), -this->_position);
    this->_position = position;
    this->setTransform(glm::translate(transform, this->_position));
}

/**
 * @brief Set transform of text block
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
 * @brief Set text align of text block
 *
 * @param textAlign Text align
 */
void TextBlock::setTextAlign(std::unique_ptr<TextAlignStrategy> textAlign) {
    this->_textAlign = std::move(textAlign);
}

/**
 * @brief Get all renderable characters in text block
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

/**
 * @brief Get number of renderable characters in text block
 *
 * @return Number of characters
 */
unsigned int TextBlock::getCharacterCount() {
    unsigned int count = 0;
    for (const TextSegment &segment : this->_segments) {
        count += segment.getCharacterCount();
    }

    return count;
}

/**
 * @brief Get the full utf-32 encoded text in text block
 *
 * @return Utf-32 text
 */
std::u32string TextBlock::getUtf32Text() {
    std::u32string text;
    for (TextSegment &segment : this->_segments) {
        text.insert(text.end(), segment.getText().begin(), segment.getText().end());
    }

    return text;
}

/**
 * @brief Get number of unicode code points in text block
 *
 * @return Number of code points
 */
unsigned int TextBlock::getCodePointCount() {
    unsigned int count = 0;
    for (const TextSegment &segment : this->_segments) {
        count += segment.getCodePointCount();
    }

    return count;
}

/**
 * @brief Get current font used in text block
 *
 * @return Current font
 */
std::shared_ptr<Font> TextBlock::getFont() const {
    return this->_font;
}

/**
 * @brief Get current font size used in text block
 *
 * @return Current font size
 */
unsigned int TextBlock::getFontSize() const {
    return this->_fontSize;
}

/**
 * @brief Get current line spacing used in text block
 *
 * @return Line spacing
 */
double TextBlock::getLineSpacing() const {
    return this->_lineSpacing;
}

/**
 * @brief Get font color used in text block
 *
 * @return Color
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
 * @brief Update all characters and use new properties of text block
 */
void TextBlock::_updateCharacters() {
    unsigned int codePointCount = this->getCodePointCount();

    for (TextSegment &segment : this->_segments) {
        this->_font = segment.getFont();
        this->_fontSize = segment.getFontSize();

        this->add(segment.getText());
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

/**
 * @brief Update renderable characters positions starting with character at given index
 *
 * @param start Index of starting character
 */
void TextBlock::_updateCharacterPositions(unsigned int start) {
    std::pair<unsigned int, LineData> firstLine = this->_lineDivider.getLineOfCharacter(start);

    // Index of first character that needs recalculating position
    // Index of first character on line
    unsigned int globalCharacterIndex = firstLine.first;

    // Restore pen position
    glm::vec2 pen{0, firstLine.second.y};
    if (this->_width > 0) {
        pen += this->_textAlign->getLineOffset(firstLine.second.width, this->_width);
    }

    // Apply calculated positions by shaper and LineData to characters
    while (globalCharacterIndex < this->getCharacterCount()) {
        auto line = this->_lineDivider.getLineOfCharacter(globalCharacterIndex);
        Character &character = this->_getCharacterBasedOnCharacterGlobalIndex(globalCharacterIndex);

        // Update pen position to start of current character
        pen += character.getOffset();

        if (line.first == globalCharacterIndex) {
            // Character is first on current line, restore pen position
            pen.x = 0;
            pen.y = line.second.y;

            // Check if text block has a width bigger than 0, that indicates to use wrapping and apply text align
            if (this->_width > 0) {
                pen += this->_textAlign->getLineOffset(line.second.width, this->_width);
            }
        }

        // Set character position
        character.setPosition(pen);
        // Update pen position to end of current character
        pen += character.getAdvance();

        globalCharacterIndex++;
    }
}

/**
 * @brief Merge text segments if they have same properties
 *
 * @param first Iterator to first segment
 * @param second Iterator to second segment
 *
 * @return Iterator to next segment
 */
std::list<TextSegment>::iterator TextBlock::_mergeSegmentsIfPossible(std::list<TextSegment>::iterator first,
                                                                     std::list<TextSegment>::iterator second) {
    if (first == this->_segments.end() || second == this->_segments.end()) {
        throw std::out_of_range("TextBlock::_mergeSegmentsIfPossible(): Invalid segment iterator");
    }

    if (first->getFont() == second->getFont() && first->getFontSize() == second->getFontSize()) {
        // Move characters from second segment to first
        first->add(second->getText());

        // Return iterator to segment one after merged segment
        return this->_segments.erase(second);
    }

    // Return iterator to segment one after second segment
    return std::next(second);
}

/**
 * @brief Get text segment based on the code point at given position
 *
 * @param index Position of code point in text block
 *
 * @return Text segment of code point
 */
TextSegment &TextBlock::_getSegmentBasedOnCodePointGlobalIndex(unsigned int index) {
    if (index > this->getCodePointCount()) {
        throw std::out_of_range(
            "TextBlock::_getSegmentBasedOnCodePointGlobalIndex(): Range exceeds available characters");
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

    throw std::runtime_error("TextBlock::_getSegmentBasedOnCodePointGlobalIndex(): Such segment does not exist");
}

/**
 * @brief Get text segment iterator based on the code point at given position
 *
 * @param index Position of code point in text block
 *
 * @return Text segment iterator of code point
 */
std::list<TextSegment>::iterator TextBlock::_getSegmentIteratorBasedOnCodePointGlobalIndex(unsigned int index) {
    if (index > this->getCodePointCount()) {
        throw std::out_of_range(
            "TextBlock::_getSegmentIteratorBasedOnCodePointGlobalIndex(): Range exceeds available characters");
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

    throw std::runtime_error(
        "TextBlock::_getSegmentIteratorBasedOnCodePointGlobalIndex(): Such segment does not exist");
}

/**
 * @brief Get text segment based on the character at given position
 *
 * @param index Position of character in text block
 *
 * @return Text segment of character
 */
TextSegment &TextBlock::_getSegmentBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index > this->getCharacterCount()) {
        throw std::out_of_range(
            "TextBlock::_getSegmentBasedOnCharacterGlobalIndex(): Range exceeds available characters");
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

    throw std::runtime_error("TextBlock::_getSegmentBasedOnCharacterGlobalIndex(): Such segment does not exist");
}

/**
 * @brief Get text segment iterator based on the character at given position
 *
 * @param index Position of character in text block
 *
 * @return Text segment iterator of character
 */
std::list<TextSegment>::iterator TextBlock::_getSegmentIteratorBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index > this->getCharacterCount()) {
        throw std::out_of_range(
            "TextBlock::_getSegmentIteratorBasedOnCharacterGlobalIndex(): Range exceeds available characters");
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

    throw std::runtime_error(
        "TextBlock::_getSegmentIteratorBasedOnCharacterGlobalIndex(): Such segment does not exist");
}

/**
 * @brief Get character at given position
 *
 * @param index Position of character in text block
 *
 * @return Character
 */
Character &TextBlock::_getCharacterBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index >= this->getCharacterCount()) {
        throw std::out_of_range(
            "TextBlock::_getCharacterBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCharacterCount()) {
            return segment.getCharacters().at(index - currentIndex);
        }

        currentIndex += segment.getCharacterCount();
    }

    throw std::runtime_error("TextBlock::_getCharacterBasedOnCharacterGlobalIndex(): Such character does not exist");
}

/**
 * @brief Get character iterator at given position
 *
 * @param index Position of character in text block
 *
 * @return Character iterator
 */
std::vector<Character>::iterator TextBlock::_getCharacterIteratorBasedOnCharacterGlobalIndex(unsigned int index) {
    if (index >= this->getCharacterCount()) {
        throw std::out_of_range(
            "TextBlock::_getCharacterIteratorBasedOnCharacterGlobalIndex(): Range exceeds available characters");
    }

    unsigned int currentIndex = 0;
    for (TextSegment &segment : this->_segments) {
        if (index < currentIndex + segment.getCharacterCount()) {
            return std::next(segment.getCharacters().begin(), index - currentIndex);
        }

        currentIndex += segment.getCharacterCount();
    }

    throw std::runtime_error(
        "TextBlock::_getCharacterIteratorBasedOnCharacterGlobalIndex(): Such character does not exist");
}

/**
 * @brief Get position of first code point in given text segment
 *
 * @param segment Given text segment
 *
 * @return Position of code point in text block
 */
unsigned int TextBlock::_getCodePointGlobalIndexBasedOnSegment(const TextSegment &segment) {
    unsigned int currentIndex = 0;
    for (const TextSegment &s : this->_segments) {
        if (&s == &segment) {
            return currentIndex;
        }

        currentIndex += s.getCodePointCount();
    }

    throw std::runtime_error("TextBlock::_getCodePointGlobalIndexBasedOnSegment(): Such segment does not exist");
}

/**
 * @brief Get position of first character in given text segment
 *
 * @param segment Given text segment
 *
 * @return Position of character in text block
 */
unsigned int TextBlock::_getCharacterGlobalIndexBasedOnSegment(const TextSegment &segment) {
    unsigned int currentIndex = 0;
    for (const TextSegment &s : this->_segments) {
        if (&s == &segment) {
            return currentIndex;
        }

        currentIndex += s.getCharacterCount();
    }

    throw std::runtime_error("TextBlock::_getCharacterGlobalIndexBasedOnSegment(): Such segment does not exist");
}

/**
 * @brief Get position of character in text block based on given character
 *
 * @param character Given character
 *
 * @return Position of character in text block
 */
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

    throw std::runtime_error("TextBlock::_getCharacterGlobalIndexBasedOnCharacter(): Such character does not exist");
}

}  // namespace vft
