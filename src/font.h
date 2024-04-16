/**
 * @file font.h
 * @author Christian Saloň
 */

#pragma once

#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "glyph.h"

namespace vft {

class Font {

private:

    std::string _fontFile;
    int _fontSize;

    FT_Library _ft;
    FT_Face _face;
    bool _supportsKerning;

    std::unordered_map<uint32_t, Glyph> _glyphInfo;

public:

    Font(std::string fontFile, int fontSize);

    bool supportsKerning() const;
    void setFontSize(int fontSize);

    void setGlyphInfo(uint32_t codePoint, Glyph glyph);
    void clearGlyphInfo();

    const Glyph &getGlyphInfo(uint32_t codePoint) const;
    const std::unordered_map<uint32_t, Glyph> &getAllGlyphInfo() const;
    std::string getFontFile() const;
    FT_Face getFace() const;
    int getFontSize() const;

private:

    void _initializeGlyphInfo();

};

}
