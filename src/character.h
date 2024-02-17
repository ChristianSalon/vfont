/**
 * @file character.h
 * @author Christian Saloň
 */

#pragma once

#include "glyph.h"

 /**
  * @class Character
  *
  * @brief Represents a character, which is rendered by vulkan
  */
class Character {

public:

    Glyph &glyph;

private:

    uint32_t _unicodeCodePoint;

    int _x;
    int _y;

    uint32_t _indexBufferOffset;
    uint32_t _vertexBufferOffset;

public:

    Character(uint32_t codePoint, Glyph &glyph, int x, int y);

    void setUnicodeCodePoint(uint32_t unicodeCodePoint);
    void setIndexBufferOffset(uint32_t indexBufferOffset);
    void setVertexBufferOffset(uint32_t vertexBufferOffset);

    uint32_t getUnicodeCodePoint();
    uint32_t getVertexBufferOffset();
    uint32_t getIndexBufferOffset();
    int getX();
    int getY();

};
