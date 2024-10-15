/**
 * @file combined_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include "tessellator.h"
#include "font.h"
#include "glyph.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "glyph_cache.h"
#include "text_renderer_utils.h"

namespace vft {

class CombinedTessellator : public Tessellator {

public:

    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;

protected:

    std::vector<Edge> _edges;

public:

    CombinedTessellator(GlyphCache& cache);
    ~CombinedTessellator() = default;

    Glyph composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) override;

protected:

    int _combineContours(std::vector<glm::vec2>& vertices, std::vector<Edge>& edges);
    int _combineContours();

    FT_Outline_MoveToFunc _getMoveToFunc() override;
    FT_Outline_LineToFunc _getLineToFunc() override;
    FT_Outline_ConicToFunc _getConicToFunc() override;

};

}
