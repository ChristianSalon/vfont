/**
 * @file character.cpp
 * @author Christian Saloň
 */

#include <glm/gtc/matrix_transform.hpp>

#include "character.h"
#include "glyph.h"

namespace vft {

/**
 * @brief Character constructor
 * 
 * @param codePoint Unicode code point
 * @param font Font of character
 * @param fontSize Font size of character
 * @param position Position of character relative to text block
 * @param transform Character transform matrix
 */
Character::Character(uint32_t codePoint, std::shared_ptr<Font> font, unsigned int fontSize, glm::vec2 position, glm::mat4 transform) : glyph{font->getGlyphInfo(codePoint)} {
    this->_unicodeCodePoint = codePoint;
    this->_position = position;
    this->_indexBufferOffset = 0;
    this->_vertexBufferOffset = 0;

    this->transform(transform, font, fontSize);
}

/**
 * @brief Setter for unicode code point
 * 
 * @param unicodeCodePoint Unicode code point of character
 */
void Character::setUnicodeCodePoint(uint32_t unicodeCodePoint) {
    this->_unicodeCodePoint = unicodeCodePoint;
}

/**
 * @brief Getter for unicode code point
 *
 * @return Unicode code point of character
 */
uint32_t Character::getUnicodeCodePoint() const {
    return this->_unicodeCodePoint;
}

/**
 * @brief Setter for vertex buffer offset
 *
 * @param vertexBufferOffset Offset in vertex buffer
 */
void Character::setVertexBufferOffset(uint32_t vertexBufferOffset) {
    this->_vertexBufferOffset = vertexBufferOffset;
}

/**
 * @brief Getter for vertex buffer offset
 *
 * @return Offset in vertex buffer
 */
uint32_t Character::getVertexBufferOffset() const {
    return this->_vertexBufferOffset;
}

/**
 * @brief Setter for index buffer offset
 *
 * @param indexBufferOffset Offset in index buffer
 */
void Character::setIndexBufferOffset(uint32_t indexBufferOffset) {
    this->_indexBufferOffset = indexBufferOffset;
}

/**
 * @brief Setter for character model matrix
 *
 * @param modelMatrix New model matrix
 */
void Character::setModelMatrix(glm::mat4 modelMatrix) {
    this->_modelMatrix = modelMatrix;
}

/**
 * @brief Set model matrix of character
 * 
 * @param transform Transform of text block
 * @param font Font
 * @param fontSize Font size of character
*/
void Character::transform(glm::mat4 transform, std::shared_ptr<Font> font, unsigned int fontSize) {
    glm::vec2 scale = font->getScalingVector(fontSize);
    this->_modelMatrix =
        transform *
        glm::translate(glm::mat4(1.f), glm::vec3(this->_position, 0.f)) *
        glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) *
        glm::scale(glm::mat4(1.f), glm::vec3(scale.x, scale.y, 0.f));
}

/**
 * @brief Getter for index buffer offset
 *
 * @return Offset in index buffer
 */
uint32_t Character::getIndexBufferOffset() const {
    return this->_indexBufferOffset;
}

/**
 * @brief Getter for character position relative to text block
 *
 * @return (X, Y) coordinates of character
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

}
