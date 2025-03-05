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
#include <glm/vec4.hpp>

#include "font.h"
#include "glyph.h"

namespace vft {

/**
 * @class Character
 *
 * @brief Represents a character which is rendered by Vulkan
 */
class Character {
protected:
    uint32_t _glyphId{0};
    uint32_t _codePoint{0}; /**< Unicode code point of character */

    std::shared_ptr<Font> _font{nullptr};
    unsigned int _fontSize{0};

    glm::vec2 _advance{0.f, 0.f};

    glm::vec2 _position{0.f, 0.f}; /**< Character position, coordinates represent the lower left corner of character */
    glm::mat4 _modelMatrix{1.f};   /**< Character model matrix */
    glm::mat4 _parentMatrix{1.f};

public:
    Character(uint32_t glyphId, uint32_t codePoint, std::shared_ptr<Font> font, unsigned int fontSize);

    void setAdvance(glm::vec2 advance);
    void setPosition(glm::vec2 position);
    void setTransform(glm::mat4 transform);

    uint32_t getGlyphId() const;
    uint32_t getCodePoint() const;
    glm::vec2 getAdvance() const;
    glm::vec2 getPosition() const;
    glm::mat4 getModelMatrix() const;
    std::shared_ptr<Font> getFont() const;
    unsigned int getFontSize() const;

protected:
    void _updateModelMatrix();
};

}  // namespace vft
