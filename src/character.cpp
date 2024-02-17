/**
 * @file character.cpp
 * @author Christian Saloň
 */

#include "character.h"
#include "glyph.h"

Character::Character(uint32_t codePoint, Glyph &glyph, int x, int y) : glyph(glyph) {
    this->_unicodeCodePoint = codePoint;
    this->_x = x;
    this->_y = y;
    this->_indexBufferOffset = 0;
    this->_vertexBufferOffset = 0;
}

void Character::setUnicodeCodePoint(uint32_t unicodeCodePoint) {
    this->_unicodeCodePoint = unicodeCodePoint;
}

uint32_t Character::getUnicodeCodePoint() {
    return this->_unicodeCodePoint;
}

void Character::setVertexBufferOffset(uint32_t vertexBufferOffset) {
    this->_vertexBufferOffset = vertexBufferOffset;
}

uint32_t Character::getVertexBufferOffset() {
    return this->_vertexBufferOffset;
}

void Character::setIndexBufferOffset(uint32_t indexBufferOffset) {
    this->_indexBufferOffset = indexBufferOffset;
}

uint32_t Character::getIndexBufferOffset() {
    return this->_indexBufferOffset;
}

int Character::getX() {
    return this->_x;
}

int Character::getY() {
    return this->_y;
}
