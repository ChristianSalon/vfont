/**
 * @file character.cpp
 * @author Christian Saloň
 */

#include "character.h"
#include "glyph.h"

/**
 * @brief Character constructor
 * 
 * @param codePoint Unicode code point
 * @param glyph Glyph data
 * @param x X coordinate of character
 * @param y Y coordinate of character
 */
Character::Character(uint32_t codePoint, Glyph &glyph, int x, int y) : glyph(glyph) {
    this->_unicodeCodePoint = codePoint;
    this->_x = x;
    this->_y = y;
    this->_indexBufferOffset = 0;
    this->_vertexBufferOffset = 0;
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
 * @brief Getter for index buffer offset
 *
 * @return Offset in index buffer
 */
uint32_t Character::getIndexBufferOffset() {
    return this->_indexBufferOffset;
}

/**
 * @brief Getter for X coordinate
 *
 * @return X coordinate of character
 */
int Character::getX() {
    return this->_x;
}

/**
 * @brief Getter for Y coordinate
 *
 * @return Y coordinate of character
 */
int Character::getY() {
    return this->_y;
}
