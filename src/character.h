/**
 * @file character.h
 * @author Christian Saloň
 */

#pragma once

#include "glyph.h"

 /**
  * @class Character
  *
  * @brief Represents a character, which is rendered by Vulkan
  */
class Character {

public:

    Glyph &glyph;                   /**< Reference to glyph data */

private:

    uint32_t _unicodeCodePoint;     /**< Unicode code point of character */

    // Coordinates represent the lower left corner of character
    int _x;                         /**< X coordinate of character */
    int _y;                         /**< Y coordinate of character */

    uint32_t _vertexBufferOffset;   /**< Offset in vertex buffer */
    uint32_t _indexBufferOffset;    /**< Offset in index buffer */

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
