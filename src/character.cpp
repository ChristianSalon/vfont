/**
 * @file character.cpp
 * @author Christian Saloň
 */

#include <glm/gtc/matrix_transform.hpp>

#include "character.h"
#include "glyph.h"

/**
 * @brief Character constructor
 * 
 * @param codePoint Unicode code point
 * @param glyph Glyph data
 * @param position Position of character
 * @param transform Character transform matrix
 */
Character::Character(uint32_t codePoint, Glyph &glyph, glm::vec3 position, glm::mat4 transform) : glyph(glyph) {
    this->_unicodeCodePoint = codePoint;
    this->_position = position;
    this->_indexBufferOffset = 0;
    this->_vertexBufferOffset = 0;

    this->_modelMatrix =
        transform *
        glm::translate(glm::mat4(1.f), this->_position) *
        glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) *
        glm::scale(glm::mat4(1.f), glm::vec3(1.f / 64.f, 1.f / 64.f, 0.f));
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
uint32_t Character::getUnicodeCodePoint() {
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
uint32_t Character::getVertexBufferOffset() {
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
 * @brief Getter for index buffer offset
 *
 * @return Offset in index buffer
 */
uint32_t Character::getIndexBufferOffset() {
    return this->_indexBufferOffset;
}

/**
 * @brief Getter for character position
 *
 * @return (X, Y, Z) coordinates of character
 */
glm::vec3 Character::getPosition() {
    return this->_position;
}

/**
 * @brief Getter for character model matrix
 *
 * @return Model matrix
 */
glm::mat4 Character::getModelMatrix() {
    return this->_modelMatrix;
}
