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
#include <glm/vec2.hpp>

namespace vft {

/**
 * @brief Represents a freetype font
 */
class Font {
public:
    static constexpr unsigned int DEFAULT_FONT_SIZE = 64; /**< Default font size used when initializing freetype font */

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
