/**
 * @file character.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "font.h"
#include "glyph.h"

namespace vft {

/**
 * @class Character
 *
 * @brief Represents a character which is rendered by Vulkan
 */
class Character {
public:
    const Glyph glyph; /**< Reference to glyph data */

protected:
    uint32_t _unicodeCodePoint; /**< Unicode code point of character */

    // Coordinates represent the lower left corner of character
    glm::vec2 _position;    /**< Character position */
    glm::mat4 _modelMatrix; /**< Character model matrix */

public:
    Character(const Glyph glyph,
              uint32_t codePoint,
              std::shared_ptr<Font> font,
              unsigned int fontSize,
              glm::vec2 position,
              glm::mat4 transform);

    void setUnicodeCodePoint(uint32_t unicodeCodePoint);
    void setModelMatrix(glm::mat4 modelMatrix);
    void transform(glm::mat4 transform, std::shared_ptr<Font> font, unsigned int fontSize);

    uint32_t getUnicodeCodePoint() const;
    glm::vec2 getPosition() const;
    glm::mat4 getModelMatrix() const;
};

}  // namespace vft
