/**
 * @file character.h
 * @author Christian Saloň
 */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "glyph.h"

 /**
  * @class Character
  *
  * @brief Represents a character, which is rendered by Vulkan
  */
class Character {

public:

    const Glyph &glyph;             /**< Reference to glyph data */

private:

    uint32_t _unicodeCodePoint;     /**< Unicode code point of character */

    // Coordinates represent the lower left corner of character
    glm::vec3 _position;            /**< Character position */
    glm::mat4 _modelMatrix;         /**< Character model matrix */

    uint32_t _vertexBufferOffset;   /**< Offset in vertex buffer */
    uint32_t _indexBufferOffset;    /**< Offset in index buffer */

public:

    Character(uint32_t codePoint, const Glyph &glyph, glm::vec3 position, glm::mat4 transform);

    void setUnicodeCodePoint(uint32_t unicodeCodePoint);
    void setVertexBufferOffset(uint32_t vertexBufferOffset);
    void setIndexBufferOffset(uint32_t indexBufferOffset);
    void setModelMatrix(glm::mat4 modelMatrix);
    void transform(glm::mat4 transform);

    uint32_t getUnicodeCodePoint() const;
    uint32_t getVertexBufferOffset() const;
    uint32_t getIndexBufferOffset() const;
    glm::vec3 getPosition() const;
    glm::mat4 getModelMatrix() const;

};
