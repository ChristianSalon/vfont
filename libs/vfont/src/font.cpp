/**
 * @file font.h
 * @author Christian Saloň
 */

#include <stdexcept>

#include "font.h"
#include "text_renderer_utils.h"

namespace vft {

/**
 * @brief Font constructor
 * 
 * @param fontFile Path to font file
 */
Font::Font(std::string fontFile) : _fontFile{fontFile} {
    if(this->_fontFile.empty()) {
        throw std::runtime_error("Path to .ttf file was not entered");
    }

    if(FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Error initializing freetype");
    }

    if(FT_New_Face(this->_ft, this->_fontFile.c_str(), 0, &(this->_face))) {
        throw std::runtime_error("Error loading font face, check path to .ttf file");
    }

    FT_Set_Pixel_Sizes(this->_face, Font::DEFAULT_FONT_SIZE, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
    this->_initializeGlyphInfo();
}

/**
 * @brief Inserts triangulated glyph to cache
 * 
 * @param codePoint Unicode cde point of glyph
 * @param glyph Triangulated glyph
 */
void Font::setGlyphInfo(uint32_t codePoint, Glyph glyph) {
    this->_glyphInfo.insert({ codePoint, glyph });
}

/**
 * @brief Resets cached glyphs
 */
void Font::clearGlyphInfo() {
    this->_glyphInfo.clear();
}

/**
 * @brief Get scaling vector based on font size. By default metrics are in size 64 pixels
 * 
 * @param fontSize Font size which is scaled to
 * 
 * @return Scale vector
 */
glm::vec2 Font::getScalingVector(unsigned int fontSize) const {
    return glm::vec2(
        (static_cast<double>(fontSize) / static_cast<double>(Font::DEFAULT_FONT_SIZE)) * (static_cast<double>(this->_face->size->metrics.x_scale) / 4194304.f),
        (static_cast<double>(fontSize) / static_cast<double>(Font::DEFAULT_FONT_SIZE)) * (static_cast<double>(this->_face->size->metrics.y_scale) / 4194304.f)
    );
}

/**
 * @brief Get cached glyph
 * 
 * @param codePoint Unicode code point of glyph
 * 
 * @return Cached glyph
 */
const Glyph &Font::getGlyphInfo(uint32_t codePoint) const {
    if(codePoint == vft::U_TAB) {
        return this->_glyphInfo.at(vft::U_SPACE);
    }

    return this->_glyphInfo.at(codePoint);
}

/**
 * @brief Get all cached glyphs
 * 
 * @return Cached glyphs
 */
const std::unordered_map<uint32_t, Glyph> &Font::getAllGlyphInfo() const {
    return this->_glyphInfo;
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
 * @brief Get path to font file
 * 
 * @return Path to font file
 */
std::string Font::getFontFile() const {
    return this->_fontFile;
}

/**
 * @brief Get handle to freetype font face
 * 
 * @return Handle to freetype font face
 */
FT_Face Font::getFace() const {
    return this->_face;
}

/**
 * @brief Set initial cache for glyphs
 */
void Font::_initializeGlyphInfo() {
    this->_glyphInfo.clear();

    this->_glyphInfo.insert({vft::U_ENTER, Glyph{}});
    this->_glyphInfo.at(vft::U_ENTER).setAdvanceX(0);
    this->_glyphInfo.at(vft::U_ENTER).setAdvanceY(this->_face->size->metrics.height);
}

}
