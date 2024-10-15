/**
 * @file cpu_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cstdint>
#include <map>

#include <glm/vec2.hpp>

#include "tessellator.h"
#include "font.h"
#include "glyph.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "glyph_cache.h"
#include "text_renderer_utils.h"

namespace vft {

class CpuTessellator : public Tessellator {

public:

    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;

protected:

    std::shared_ptr<Font> _font;
    unsigned int _fontSize;
    std::vector<Edge> _edges;

public:

    CpuTessellator(GlyphCache& cache);
    ~CpuTessellator() = default;

    Glyph composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) override;

protected:

    int _combineContours(std::vector<glm::vec2> &vertices, std::vector<Edge> &edges);
    int _combineContours();

    std::set<float> _subdivideQuadraticBezier(const std::array<glm::vec2, 3> curve);
    void _subdivide(
        const std::array<glm::vec2, 3> curve,
        float t,
        float delta,
        std::set<float>& newVertices);

    FT_Outline_MoveToFunc _getMoveToFunc() override;
    FT_Outline_LineToFunc _getLineToFunc() override;
    FT_Outline_ConicToFunc _getConicToFunc() override;

};

}
