/**
 * @file text_block.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "character.h"
#include "font.h"
#include "tessellator.h"

namespace vft {

/**
 * @class TextBlock
 *
 * @brief Groups together characters which are rendered
 */
class TextBlock {
public:
    std::function<void()> onTextChange; /**< Callback when text in block changes */

protected:
    std::shared_ptr<Font> _font; /**< Font of text in text block */

    bool _kerning;          /**< Indicates whether to use kerning in text block */
    bool _wrapping;         /**< Indicates whether to use wrapping in text block */
    unsigned int _fontSize; /**< Font size in text block */
    int _width;             /**< Width of text block. -1 indicates unlimited width */
    int _penX;              /**< X coordinate of current pen position */
    int _penY;              /**< Y coordinate of current pen position */

    glm::mat4 _transform; /**< Transform matrix of text block */
    glm::vec4 _color;     /**< Color of text */
    glm::vec3 _position;  /**< Position of text block */

    std::vector<Character> _characters; /**< Characters to render */
    std::shared_ptr<Tessellator> _tessellator;

public:
    TextBlock(std::shared_ptr<Font> font,
              unsigned int fontSize,
              glm::vec4 color,
              glm::vec3 position,
              int width = -1,
              bool kerning = true,
              bool wrapping = false);

    void scale(float x, float y, float z);
    void translate(float x, float y, float z);
    void rotate(float x, float y, float z);

    void add(std::vector<uint32_t> codePoints);
    void add(std::string text);
    void remove(unsigned int count = 1);
    void clear();

    void setFont(std::shared_ptr<Font> font);
    void setColor(glm::vec4 color);
    void setPosition(glm::vec3 position);
    void setTransform(glm::mat4 transform);
    void setWidth(int width);
    void setKerning(bool kerning);
    void setWrapping(bool wrapping);
    void setFontSize(unsigned int fontSize);
    void setTessellationStrategy(std::shared_ptr<Tessellator> tessellator);

    std::vector<Character> &getCharacters();
    std::shared_ptr<Font> getFont() const;
    glm::vec4 getColor() const;
    glm::vec3 getPosition() const;
    glm::mat4 getTransform() const;
    int getWidth() const;
    bool getKerning() const;
    bool getWrapping() const;
    unsigned int getFontSize() const;

protected:
    void _updateCharacters();
    void _updateTransform();
};

}  // namespace vft
