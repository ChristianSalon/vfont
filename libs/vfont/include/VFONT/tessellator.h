/**
 * @file tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <glm/vec2.hpp>

#include "font.h"
#include "glyph.h"
#include "text_renderer_utils.h"

namespace vft {

class Tessellator {
protected:
    FT_Outline_MoveToFunc _moveToFunc{nullptr};
    FT_Outline_LineToFunc _lineToFunc{nullptr};
    FT_Outline_ConicToFunc _conicToFunc{nullptr};
    FT_Outline_CubicToFunc _cubicToFunc{nullptr};

    Glyph _currentGlyph{};                 /**< Glyph that is currently being composed */
    ComposedGlyphData _currentGlyphData{}; /**< Data of glyph that is currently being composed */

public:
    Tessellator();
    ~Tessellator() = default;

    virtual Glyph composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize = 0) = 0;

protected:
    Glyph _composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font);
    uint32_t _getVertexIndex(const glm::vec2 &vertex);
};

}  // namespace vft
