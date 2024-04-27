/**
 * @file font.h
 * @author Christian Saloň
 */

#include <stdexcept>

#include "font.h"
#include "text_renderer_utils.h"

namespace vft {

Font::Font(std::string fontFile, int fontSize) : _fontFile{fontFile}, _fontSize{fontSize} {
    if(this->_fontFile.empty()) {
        throw std::runtime_error("Path to .ttf file was not entered");
    }

    if(FT_Init_FreeType(&(this->_ft))) {
        throw std::runtime_error("Error initializing freetype");
    }

    if(FT_New_Face(this->_ft, this->_fontFile.c_str(), 0, &(this->_face))) {
        throw std::runtime_error("Error loading font face, check path to .ttf file");
    }

    FT_Set_Pixel_Sizes(this->_face, this->_fontSize, 0);

    this->_supportsKerning = FT_HAS_KERNING(this->_face);
    this->_initializeGlyphInfo();
}

void Font::setGlyphInfo(uint32_t codePoint, Glyph glyph) {
    this->_glyphInfo.insert({ codePoint, glyph });
}

void Font::setFontSize(int fontSize) {
    this->_fontSize = fontSize;
    FT_Set_Pixel_Sizes(this->_face, this->_fontSize, 0);

    this->_initializeGlyphInfo();
}

void Font::clearGlyphInfo() {
    this->_glyphInfo.clear();
}

const Glyph &Font::getGlyphInfo(uint32_t codePoint) const {
    return this->_glyphInfo.at(codePoint);
}

const std::unordered_map<uint32_t, Glyph> &Font::getAllGlyphInfo() const {
    return this->_glyphInfo;
}

bool Font::supportsKerning() const {
    return this->_supportsKerning;
}

std::string Font::getFontFile() const {
    return this->_fontFile;
}

FT_Face Font::getFace() const {
    return this->_face;
}

int Font::getFontSize() const {
    return this->_fontSize;
}

void Font::_initializeGlyphInfo() {
    this->_glyphInfo.clear();

    this->_glyphInfo.insert({vft::U_ENTER, Glyph{}});
    this->_glyphInfo.at(vft::U_ENTER).setAdvanceX(0);
    this->_glyphInfo.at(vft::U_ENTER).setAdvanceY(this->_fontSize);
}

}
