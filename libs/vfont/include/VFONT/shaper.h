/**
 * @file shaper.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <hb-ft.h>
#include <hb.h>

#include "font.h"
#include "unicode.h"

namespace vft {

/**
 * @brief Character data after shaping using harfbuzz
 */
typedef struct {
    uint32_t glyphId; /**< Glyph id of shaped character */
    uint32_t cluster; /**< Cluster id of shaped character (see harfbuzz clusters) */
    double xAdvance;  /**< X advance of shaped character */
    double yAdvance;  /**< Y advance of shaped character */
    double xOffset;   /**< X offset of shaped characer */
    double yOffset;   /**< Y offset of shaped character */
} ShapedCharacter;

/**
 * @brief Shapes text using harfbuzz
 */
class Shaper {
public:
    static std::vector<std::vector<ShapedCharacter>> shape(std::vector<uint32_t> codePoints,
                                                           std::shared_ptr<Font> font,
                                                           unsigned int fontSize);

protected:
    static void _preprocessInput(std::vector<uint32_t> &codePoints);
};

}  // namespace vft
