/**
 * @file font.cpp
 * @author Christian Saloň
 */

#include "font.h"

namespace vft {

/**
 * @brief Font constructor, loads font from fontt file
 *
 * @param fontFile Path to font file
 */
Font::Font(std::string fontFile) {
    if (fontFile.empty()) {
        throw std::runtime_error("Path to .ttf file was not entered");
    }

    if (FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Error initializing freetype");
    }

    if (FT_New_Face(this->_ft, fontFile.c_str(), 0, &this->_face)) {
        throw std::runtime_error("Error loading font face, check path to .ttf file");
    }

    FT_Set_Pixel_Sizes(this->_face, Font::DEFAULT_FONT_SIZE, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
}

/**
 * @brief Font constructor, loads font from memory
 *
 * @param fontFile Path to font file
 */
Font::Font(uint8_t *buffer, long size) {
    if (size <= 0) {
        throw std::runtime_error("Buffer size must be greater than zero");
    }

    if (FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Error initializing freetype");
    }

    if (FT_New_Memory_Face(this->_ft, buffer, size, 0, &this->_face)) {
        throw std::runtime_error("Error loading font from memory");
    }

    FT_Set_Pixel_Sizes(this->_face, Font::DEFAULT_FONT_SIZE, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
}

/**
 * @brief Get scaling vector based on font size. By default metrics are in size 64 pixels
 *
 * @param fontSize Font size which is scaled to
 *
 * @return Scale vector
 */
glm::vec2 Font::getScalingVector(unsigned int fontSize) const {
    return glm::vec2((static_cast<double>(fontSize) / static_cast<double>(Font::DEFAULT_FONT_SIZE)) *
                         (static_cast<double>(this->_face->size->metrics.x_scale) / 4194304.f),
                     (static_cast<double>(fontSize) / static_cast<double>(Font::DEFAULT_FONT_SIZE)) *
                         (static_cast<double>(this->_face->size->metrics.y_scale) / 4194304.f));
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
 * @brief Get font family name
 *
 * @return Font family name
 */
std::string Font::getFontFamily() const {
    return std::string(this->_face->family_name);
}

/**
 * @brief Get handle to freetype font face
 *
 * @return Handle to freetype font face
 */
FT_Face Font::getFace() const {
    return this->_face;
}

}  // namespace vft
