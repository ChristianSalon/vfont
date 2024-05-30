/**
 * @file font.h
 * @author Christian Saloň
 */

#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "glyph.h"

namespace vft {

/**
 * @class Font
 *
 * @brief Represents a font and stores triangulated glyphs in cache
 */
class Font {

public:

    static constexpr unsigned int DEFAULT_FONT_SIZE = 64;

protected:

    std::string _fontFile;                          /**< Path to font file */

    FT_Library _ft;                                 /**< Freetype library */
    FT_Face _face;                                  /**< Freetype font face */
    bool _supportsKerning;                          /**< Indicates whether this font supprots kerning. */

    std::unordered_map<uint32_t, Glyph> _glyphInfo; /**< Cached triangulated glyphs. Key: Unicode code point */

public:

    Font(std::string fontFile);

    void setGlyphInfo(uint32_t codePoint, Glyph glyph);
    void clearGlyphInfo();

    bool supportsKerning() const;
    glm::vec2 getScalingVector(unsigned int fontSize) const;
    const Glyph &getGlyphInfo(uint32_t codePoint) const;
    const std::unordered_map<uint32_t, Glyph> &getAllGlyphInfo() const;
    std::string getFontFile() const;
    FT_Face getFace() const;

protected:

    void _initializeGlyphInfo();

};

}
