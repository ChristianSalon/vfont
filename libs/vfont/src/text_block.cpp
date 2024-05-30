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

/**
 * @brief TextBloc constructor
 * 
 * @param font Font used in text block
 * @param fontSize Font size in pixels
 * @param color Font color
 * @param position Position of text block in scene
 * @param width Width of text block. -1 indicates unlimited width
 * @param kerning Indicates whether to use kerning in block
 * @param wrapping Indicates whether to use wrapping in block
 */
TextBlock::TextBlock(
    std::shared_ptr<Font> font,
    unsigned int fontSize,
    glm::vec3 color,
    glm::vec3 position,
    int width,
    bool kerning,
    bool wrapping) : _font{font},
                     _fontSize{fontSize},
                    _penX{0},
                    _penY{static_cast<int>(fontSize)},
                    _transform{glm::mat4(1.f)},
                    _position{glm::vec3(0, 0, 0)} {
    this->setKerning(kerning);
    this->setWrapping(wrapping);
    this->setWidth(width);
    this->setColor(color);
    this->setPosition(position);
 }

/**
 * @brief Apply scale to text block
 * 
 * @param x Scale in X direction
 * @param y Scale in Y direction
 * @param z Scale in Z direction
 */
void TextBlock::scale(float x, float y, float z) {
    this->setTransform(glm::scale(this->getTransform(), glm::vec3(x, y, z)));
}

/**
 * @brief Apply translation to text block
 *
 * @param x Translation in X direction
 * @param y Translation in Y direction
 * @param z Translation in Z direction
 */
void TextBlock::translate(float x, float y, float z) {
    this->setTransform(glm::translate(this->getTransform(), glm::vec3(x, y, z)));
}

/**
 * @brief Apply rotation to text block
 *
 * @param x Rotation around the X axis
 * @param y Rotation around the Y axis
 * @param z Rotation around the Z axis
 */
void TextBlock::rotate(float x, float y, float z) {
    glm::mat4 newMatrix = this->getTransform();
    newMatrix = glm::rotate(newMatrix, glm::radians(x), glm::vec3(1.f, 0.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(y), glm::vec3(0.f, 1.f, 0.f));
    newMatrix = glm::rotate(newMatrix, glm::radians(z), glm::vec3(0.f, 0.f, 1.f));

    this->setTransform(newMatrix);
}

/**
 * @brief Add characters to block
 * 
 * @param codePoints Unicode code points of characters
 */
void TextBlock::add(std::vector<uint32_t> codePoints) {
    for(uint32_t codePoint : codePoints) {
        // Creates glyph info if not set
        GlyphCompositor compositor;
        compositor.compose(codePoint, this->_font);

        if(codePoint == vft::U_ENTER) {
            this->_penX = 0;
            this->_penY += this->getFontSize();

            this->_characters.push_back(Character(codePoint, this->_font, this->_fontSize, glm::vec3(this->_penX, this->_penY, 0), this->_transform));
        }
        else if(codePoint == vft::U_TAB) {
            this->_characters.push_back(Character(codePoint, this->_font, this->_fontSize, glm::vec3(this->_penX, this->_penY, 0), this->_transform));

            glm::vec2 scale = this->_font->getScalingVector(this->_fontSize);
            for(int i = 0; i < 4; i++) {
                this->_penX += this->_characters.back().glyph.getAdvanceX() * scale.x;
                this->_penY += this->_characters.back().glyph.getAdvanceY() * scale.y;
            }
        }
        else {
            // Apply wrapping if specified
            bool wasWrapped = false;
            if(this->_width >= 0 && this->_penX + this->_font->getGlyphInfo(codePoint).getAdvanceX() > this->getWidth()) {
                // Render character on new line by adding enter character
                wasWrapped = true;
                this->_penX = 0;
                this->_penY += this->getFontSize();
            }
            
            glm::vec2 scale = this->_font->getScalingVector(this->_fontSize);

            // Apply kerning if specified
            if(!wasWrapped && this->_kerning && this->_characters.size() > 0) {
                uint32_t leftCharIndex = FT_Get_Char_Index(this->_font->getFace(), this->_characters.back().getUnicodeCodePoint());
                uint32_t rightCharIndex = FT_Get_Char_Index(this->_font->getFace(), codePoint);


                FT_Vector delta;
                FT_Get_Kerning(this->_font->getFace(), leftCharIndex, rightCharIndex, FT_KERNING_UNSCALED, &delta);
                this->_penX += delta.x * scale.x;
            }

            this->_characters.push_back(Character(codePoint, this->_font, this->_fontSize, glm::vec3(this->_penX, this->_penY, 0), this->_transform));

            this->_penX += this->_characters.back().glyph.getAdvanceX() * scale.x;
            this->_penY += this->_characters.back().glyph.getAdvanceY() * scale.y;
        }

        if(this->onTextChange) {
            this->onTextChange();
        }
    }
}

/**
 * @brief Remove characters from end of text block
 * 
 * @param count Amount of characters to remove
 */
void TextBlock::remove(unsigned int count) {
    for(unsigned int i = 0; i < count; i++) {
        if(this->_characters.size() == 0) {
            return;
        }
        else if(this->_characters.size() == 1) {
            this->_characters.pop_back();
            this->_penX = 0;
            this->_penY = this->getFontSize();

            return;
        }

        glm::vec2 scale = this->_font->getScalingVector(this->_fontSize);

        if(this->_characters.back().getUnicodeCodePoint() == vft::U_ENTER) {
            this->_characters.pop_back();

            this->_penX = this->_characters.back().getPosition().x + this->_characters.back().glyph.getAdvanceX() * scale.x;
            this->_penY -= this->getFontSize();
        }
        else {
            this->_characters.pop_back();

            if(this->_characters.back().getUnicodeCodePoint() == vft::U_TAB) {
                this->_penX = this->_characters.back().getPosition().x;
                this->_penY = this->_characters.back().getPosition().y;

                for(int i = 0; i < 4; i++) {
                    this->_penX += this->_characters.back().glyph.getAdvanceX() * scale.x;
                }
            }
            else {
                this->_penX = this->_characters.back().getPosition().x + this->_characters.back().glyph.getAdvanceX() * scale.x;
                this->_penY = this->_characters.back().getPosition().y;
            }
        }
    }

    if(this->onTextChange) {
        this->onTextChange();
    }
}

/**
 * @brief Remove all characters in text block
 */
void TextBlock::clear() {
    this->_characters.clear();
}

/**
 * @brief Set font in text block
 * 
 * @param font Font
 */
void TextBlock::setFont(std::shared_ptr<Font> font) {
    this->_font = font;
    this->_updateCharacters();
}

/**
 * @brief Set font size of characters in text block
 * 
 * @param fontSize Font size in pixels
*/
void TextBlock::setFontSize(unsigned int fontSize) {
    this->_fontSize = fontSize;
    this->_updateCharacters();
}

/**
 * @brief Set color of characters in text block
 * 
 * @param color Color
 */
void TextBlock::setColor(glm::vec3 color) {
    this->_color = color;
}

/**
 * @brief Set position of text block in scene
 * 
 * @param position 3D position
 */
void TextBlock::setPosition(glm::vec3 position) {
    glm::mat4 transform = glm::translate(this->getTransform(), -this->_position);
    this->_position = position;
    this->setTransform(glm::translate(transform, this->_position));
}

/**
 * @brief Set transform of text block and update characters
 * 
 * @param transform Transform matrix
 */
void TextBlock::setTransform(glm::mat4 transform) {
    this->_transform = transform;
    this->_updateTransform();
}

/**
 * @brief Set width of text block. -1 indicates unlimited width
 * 
 * @param width Width of text block
 */
void TextBlock::setWidth(int width) {
    this->_width = width;
    this->_updateCharacters();
}

/**
 * @brief Check if font supports kerning, apply if supported and update characters
 * 
 * @param kerning Indicates whether to use kerning in text block
 */
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

/**
 * @brief Set whether to use wrapping in text block
 *
 * @param kerning Indicates whether to use wrapping in text block
 */
void TextBlock::setWrapping(bool wrapping) {
    if(this->getWrapping() != wrapping) {
        this->_wrapping = wrapping;
        this->_updateCharacters();
    }
}

/**
 * @brief Get all characters in text block
 * 
 * @return Characters in text block
 */
std::vector<Character> &TextBlock::getCharacters() {
    return this->_characters;
}

/**
 * @brief Get font used in text block
 * 
 * @return Font used in text block
 */
std::shared_ptr<Font> TextBlock::getFont() const {
    return this->_font;
}

/**
 * @brief Get font size used in text block
 *
 * @return Font size used in text block
 */
unsigned int TextBlock::getFontSize() const {
    return this->_fontSize;
}

/**
 * @brief Get font color used in text block
 *
 * @return Font color used in text block
 */
glm::vec3 TextBlock::getColor() const {
    return this->_color;
}

/**
 * @brief Get position of text block
 *
 * @return Position of text block
 */
glm::vec3 TextBlock::getPosition() const {
    return this->_position;
}

/**
 * @brief Get width of text block
 *
 * @return Width of text block
 */
int TextBlock::getWidth() const {
    return this->_width;
}

/**
 * @brief Get transform of text block
 *
 * @return Transform matrix
 */
glm::mat4 TextBlock::getTransform() const {
    return this->_transform;
}

/**
 * @brief Get whether kerning is used in text block
 *
 * @return True if kerning is used, else false
 */
bool TextBlock::getKerning() const {
    return this->_kerning;
}

/**
 * @brief Get whether wrapping is used in text block
 *
 * @return True if wrapping is used, else false
 */
bool TextBlock::getWrapping() const {
    return this->_wrapping;
}

/**
 * @brief Update all characetrs and use new properties of text block
 */
void TextBlock::_updateCharacters() {
    std::vector<uint32_t> codePoints;
    for(Character &character : this->_characters) {
        codePoints.push_back(character.getUnicodeCodePoint());
    }

    this->_characters.clear();
    this->add(codePoints);
}

/**
 * @brief Update all characters and use new transform of text block
 */
void TextBlock::_updateTransform() {
    for(Character &character : this->_characters) {
        character.transform(this->getTransform(), this->_font, this->_fontSize);
    }
}

}
