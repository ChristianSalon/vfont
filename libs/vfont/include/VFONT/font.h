/**
 * @file font.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

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
    FT_Library _ft;        /**< Freetype library */
    FT_Face _face;         /**< Freetype font face */
    bool _supportsKerning; /**< Indicates whether this font supprots kerning. */

public:
    Font(std::string fontFile);
    Font(uint8_t *buffer, long size);

    bool supportsKerning() const;
    std::string getFontFamily() const;
    glm::vec2 getScalingVector(unsigned int fontSize) const;
    FT_Face getFace() const;
};

}  // namespace vft
