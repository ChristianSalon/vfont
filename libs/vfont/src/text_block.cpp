/**
 * @file text_block.h
 * @author Christian Saloň
 */

#include <iostream>

#include <glm/ext/matrix_transform.hpp>

#include "text_block.h"
#include "text_renderer.h"
#include "text_renderer_utils.h"
#include "glyph_compositor.h"


namespace vft {

TextBlock::TextBlock(
    std::shared_ptr<Font> font,
    glm::vec3 color,
    glm::vec3 position,
    int width,
    bool kerning,
    bool wrapping) : _font{font},
                    _penX{0},
                    _penY{this->_font->getFontSize()},
                    _transform{glm::mat4(1.f)},
                    _position{glm::vec3(0, 0, 0)} {
    this->setKerning(kerning);
    this->setWrapping(wrapping);
    this->setWidth(width);
    this->setColor(color);
    this->setPosition(position);
 }

void TextBlock::scale(float x, float y, float z) {
    this->setTransform(glm::scale(this->getTransform(), glm::vec3(x, y, z)));
}

void TextBlock::translate(float x, float y, float z) {
    this->setTransform(glm::translate(this->getTransform(), glm::vec3(x, y, z)));
}

void TextBlock::rotate(float x, float y, float z) {
    glm::mat4 newMatrix = this->getTransform();
    newMatrix = glm::rotate(newMatrix, glm::radians(x), glm::vec3(1.f, 0.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(y), glm::vec3(0.f, 1.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(z), glm::vec3(0.f, 0.f, 1.f));

    this->setTransform(newMatrix);
}

void TextBlock::add(std::vector<uint32_t> codePoints) {
    for(uint32_t codePoint : codePoints) {
        // Creates glyph info if not set
        GlyphCompositor compositor;
        compositor.compose(codePoint, this->_font);

        if(codePoint == vft::U_ENTER) {
            this->_penX = 0;
            this->_penY += this->_font->getFontSize();

            this->_characters.push_back(Character(codePoint, this->_font->getGlyphInfo(codePoint), glm::vec3(this->_penX, this->_penY, 0), this->_transform));
        }
        else if(codePoint == vft::U_TAB) {
            this->_characters.push_back(Character(codePoint, this->_font->getGlyphInfo(vft::U_SPACE), glm::vec3(this->_penX, this->_penY, 0), this->_transform));

            for(int i = 0; i < 4; i++) {
                this->_penX += this->_characters.back().glyph.getAdvanceX();
                this->_penY += this->_characters.back().glyph.getAdvanceY();
            }
        }
        else {
            // Apply wrapping if specified
            bool wasWrapped = false;
            if(this->_width >= 0 && this->_penX + this->_font->getGlyphInfo(codePoint).getAdvanceX() > this->getWidth()) {
                // Render character on new line by adding enter character
                wasWrapped = true;
                this->_penX = 0;
                this->_penY += this->_font->getFontSize();
            }

            // Apply kerning if specified
            if(!wasWrapped && this->_kerning && this->_characters.size() > 0) {
                uint32_t leftCharIndex = FT_Get_Char_Index(this->_font->getFace(), this->_characters.back().getUnicodeCodePoint());
                uint32_t rightCharIndex = FT_Get_Char_Index(this->_font->getFace(), codePoint);

                FT_Vector delta;
                FT_Get_Kerning(this->_font->getFace(), leftCharIndex, rightCharIndex, FT_KERNING_DEFAULT, &delta);
                this->_penX += delta.x >> 6;
            }

            this->_characters.push_back(Character(codePoint, this->_font->getGlyphInfo(codePoint), glm::vec3(this->_penX, this->_penY, 0), this->_transform));

            this->_penX += this->_characters.back().glyph.getAdvanceX();
            this->_penY += this->_characters.back().glyph.getAdvanceY();
        }

        if(this->onTextChange) {
            this->onTextChange();
        }
    }
}

void TextBlock::remove(unsigned int count) {
    for(unsigned int i = 0; i < count; i++) {
        if(this->_characters.size() == 0) {
            return;
        }
        else if(this->_characters.size() == 1) {
            this->_characters.pop_back();
            this->_penX = 0;
            this->_penY = this->_font->getFontSize();

            return;
        }

        if(this->_characters.back().getUnicodeCodePoint() == vft::U_ENTER) {
            this->_characters.pop_back();

            this->_penX = this->_characters.back().getPosition().x + this->_characters.back().glyph.getAdvanceX();
            this->_penY -= this->_font->getFontSize();
        }
        else {
            this->_characters.pop_back();

            if(this->_characters.back().getUnicodeCodePoint() == vft::U_TAB) {
                this->_penX = this->_characters.back().getPosition().x;
                this->_penY = this->_characters.back().getPosition().y;

                for(int i = 0; i < 4; i++) {
                    this->_penX += this->_characters.back().glyph.getAdvanceX();
                }
            }
            else {
                this->_penX = this->_characters.back().getPosition().x + this->_characters.back().glyph.getAdvanceX();
                this->_penY = this->_characters.back().getPosition().y;
            }
        }
    }

    if(this->onTextChange) {
        this->onTextChange();
    }
}

void TextBlock::clear() {
    this->_characters.clear();
}

void TextBlock::setFont(std::shared_ptr<Font> font) {
    this->_font = font;
    this->_updateCharacters();
}

void TextBlock::setColor(glm::vec3 color) {
    this->_color = color;
}

void TextBlock::setPosition(glm::vec3 position) {
    glm::mat4 transform = glm::translate(this->getTransform(), -this->_position);
    this->_position = position;
    this->setTransform(glm::translate(transform, this->_position));
}

void TextBlock::setTransform(glm::mat4 transform) {
    this->_transform = transform;
    this->_updateTransform();
}

void TextBlock::setWidth(int width) {
    this->_width = width;
    this->_updateCharacters();
}

void TextBlock::setKerning(bool kerning) {
    if(kerning && !this->_font->supportsKerning()) {
        std::cerr << "Font " << this->_font->getFontFile() << " does not support kerning" << std::endl;
        return;
    }

    if(this->getKerning() != kerning) {
        this->_kerning = kerning;
        this->_updateCharacters();
    }
}

void TextBlock::setWrapping(bool wrapping) {
    if(this->getWrapping() != wrapping) {
        this->_wrapping = wrapping;
        this->_updateCharacters();
    }
}

std::vector<Character> &TextBlock::getCharacters() {
    return this->_characters;
}

std::shared_ptr<Font> TextBlock::getFont() const {
    return this->_font;
}

glm::vec3 TextBlock::getColor() const {
    return this->_color;
}

glm::vec3 TextBlock::getPosition() const {
    return this->_position;
}

int TextBlock::getWidth() const {
    return this->_width;
}

glm::mat4 TextBlock::getTransform() const {
    return this->_transform;
}

bool TextBlock::getKerning() const {
    return this->_kerning;
}

bool TextBlock::getWrapping() const {
    return this->_wrapping;
}

void TextBlock::_updateCharacters() {
    std::vector<uint32_t> codePoints;
    for(Character &character : this->_characters) {
        codePoints.push_back(character.getUnicodeCodePoint());
    }

    this->_characters.clear();
    this->add(codePoints);
}

void TextBlock::_updateTransform() {
    for(Character &character : this->_characters) {
        character.transform(this->getTransform());
    }
}

}
