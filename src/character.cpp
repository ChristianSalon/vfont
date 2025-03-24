/**
 * @file character.cpp
 * @author Christian Saloň
 */

#include "character.h"

namespace vft {

/**
 * @brief Character constructor
 *
 * @param glyphId Glyph id of character
 * @param codePoint Unicode code point
 * @param font Font of character
 * @param fontSize Font size of character
 */
Character::Character(uint32_t glyphId, uint32_t codePoint, std::shared_ptr<Font> font, unsigned int fontSize)
    : _glyphId{glyphId}, _codePoint{codePoint}, _font{font}, _fontSize{fontSize} {
    this->_updateModelMatrix();
}

/**
 * @brief Setter for character advance
 *
 * @param advance Advance in pixels
 */
void Character::setAdvance(glm::vec2 advance) {
    this->_advance = advance;
}

/**
 * @brief Setter for character position in text block
 *
 * @param position Position in text block
 */
void Character::setPosition(glm::vec2 position) {
    this->_position = position;
    this->_updateModelMatrix();
}

/**
 * @brief Setter for the transform of text block
 *
 * @param transform Transform matrix
 */
void Character::setTransform(glm::mat4 transform) {
    this->_parentMatrix = transform;
    this->_updateModelMatrix();
}

/**
 * @brief Getter for glyph id
 *
 * @return Glyph id of character
 */
uint32_t Character::getGlyphId() const {
    return this->_glyphId;
}

/**
 * @brief Getter for unicode code point of character
 *
 * @return Unicode code point
 */
uint32_t Character::getCodePoint() const {
    return this->_codePoint;
}

/**
 * @brief Getter for character advance
 *
 * @return Advance vector
 */
glm::vec2 Character::getAdvance() const {
    return this->_advance;
}

/**
 * @brief Getter for character position relative to text block
 *
 * @return Character position
 */
glm::vec2 Character::getPosition() const {
    return this->_position;
}

/**
 * @brief Getter for character model matrix
 *
 * @return Model matrix
 */
glm::mat4 Character::getModelMatrix() const {
    return this->_modelMatrix;
}

/**
 * @brief Getter for the charater's font
 * @return Font used by character
 */
std::shared_ptr<Font> Character::getFont() const {
    return this->_font;
}

/**
 * @brief Getter for font size of character
 * @return Font size
 */
unsigned int Character::getFontSize() const {
    return this->_fontSize;
}

/**
 * @brief Updates the model matrix of character
 */
void Character::_updateModelMatrix() {
    glm::vec2 scale = this->_font->getScalingVector(this->_fontSize);
    this->_modelMatrix = this->_parentMatrix * glm::translate(glm::mat4(1.f), glm::vec3(this->_position, 0.f)) *
                         glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) *
                         glm::scale(glm::mat4(1.f), glm::vec3(scale.x, scale.y, 0.f));
}

}  // namespace vft
