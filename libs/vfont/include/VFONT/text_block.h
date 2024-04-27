/**
 * @file text_block.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "character.h"
#include "font.h"

namespace vft {

class TextBlock {

public:

    std::function<void()> onTextChange;

protected:

    std::shared_ptr<Font> _font;

    bool _kerning;
    bool _wrapping;
    int _width;
    int _penX;
    int _penY;

    glm::mat4 _transform;
    glm::vec3 _color;
    glm::vec3 _position;

    std::vector<Character> _characters;

public:

    TextBlock(std::shared_ptr<Font> font,
              glm::vec3 color,
              glm::vec3 position,
              int width = -1,
              bool kerning = true,
              bool wrapping = false);

    void scale(float x, float y, float z);
    void translate(float x, float y, float z);
    void rotate(float x, float y, float z);

    void add(std::vector<uint32_t> codePoints);
    void remove(unsigned int count = 1);
    void clear();

    void setFont(std::shared_ptr<Font> font);
    void setColor(glm::vec3 color);
    void setPosition(glm::vec3 position);
    void setTransform(glm::mat4 transform);
    void setWidth(int width);
    void setKerning(bool kerning);
    void setWrapping(bool wrapping);

    std::vector<Character> &getCharacters();
    std::shared_ptr<Font> getFont() const;
    glm::vec3 getColor() const;
    glm::vec3 getPosition() const;
    glm::mat4 getTransform() const;
    int getWidth() const;
    bool getKerning() const;
    bool getWrapping() const;

protected:

    void _updateCharacters();
    void _updateTransform();

};

}
