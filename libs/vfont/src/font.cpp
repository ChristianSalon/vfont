/**
 * @file font.cpp
 * @author Christian Saloň
 */

#include "font.h"

namespace vft {

/**
 * @brief Font constructor, loads font from font file
 *
 * @param fontFile Path to font file
 */
Font::Font(std::string fontFile) {
    if (fontFile.empty()) {
        throw std::runtime_error("Font::Font(): Path to .ttf file was not entered");
    }

    if (FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Font::Font(): Error initializing freetype");
    }

    if (FT_New_Face(this->_ft, fontFile.c_str(), 0, &this->_face)) {
        throw std::runtime_error("Font::Font(): Error loading font face, check path to .ttf file");
    }

    FT_Set_Pixel_Sizes(this->_face, this->_pixelSize, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
}

/**
 * @brief Font constructor, loads font from memory
 *
 * @param buffer Pointer to memory where the font is stored
 * @param size Size of buffer
 */
Font::Font(uint8_t *buffer, long size) {
    if (size <= 0) {
        throw std::runtime_error("Font::Font(): Buffer size must be greater than zero");
    }

    if (FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Font::Font(): Error initializing freetype");
    }

    if (FT_New_Memory_Face(this->_ft, buffer, size, 0, &this->_face)) {
        throw std::runtime_error("Font::Font(): Error loading font from memory");
    }

    FT_Set_Pixel_Sizes(this->_face, this->_pixelSize, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
}

void Font::setPixelSize(unsigned int pixelSize) {
    this->_pixelSize = pixelSize;
    FT_Set_Pixel_Sizes(this->_face, this->_pixelSize, this->_pixelSize);
}

/**
 * @brief Get scaling vector based on font size. Used for converting from font units to pixels
 *
 * @param fontSize Font size
 *
 * @return Scale vector
 */
glm::vec2 Font::getScalingVector(unsigned int fontSize) const {
    return glm::vec2((static_cast<double>(fontSize) / static_cast<double>(this->_pixelSize)) *
                         (static_cast<double>(this->_face->size->metrics.x_scale) / 4194304.f),
                     (static_cast<double>(fontSize) / static_cast<double>(this->_pixelSize)) *
                         (static_cast<double>(this->_face->size->metrics.y_scale) / 4194304.f));
}

unsigned int Font::getPixelSize() const {
    return this->_pixelSize;
}

/**
 * @brief Indicates whether font supports kerning
 *
 * @return True if suports kerning, else false
 */
bool Font::supportsKerning() const {
    return this->_supportsKerning;
}

/**
 * @brief Getter for font family name
 *
 * @return Font family name
 */
std::string Font::getFontFamily() const {
    return std::string(this->_face->family_name);
}

/**
 * @brief Getter for freetype font face
 *
 * @return Handle to freetype font face
 */
FT_Face Font::getFace() const {
    return this->_face;
}

}  // namespace vft
