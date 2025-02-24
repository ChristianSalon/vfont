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

namespace vft {

typedef struct {
    uint32_t glyphId;
    uint32_t cluster;
    double xAdvance;
    double yAdvance;
    double xOffset;
    double yOffset;
} ShapedCharacter;

/**
 * @class Shaper
 *
 * @brief Shapes input text using harfbuzz
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
