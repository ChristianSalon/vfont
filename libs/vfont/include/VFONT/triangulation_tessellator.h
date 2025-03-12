/**
 * @file triangulation_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

#include <glm/vec2.hpp>

#include "circular_dll.h"
#include "font.h"
#include "glyph.h"
#include "glyph_cache.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "polygon_operator.h"
#include "tessellator.h"
#include "text_renderer_utils.h"

namespace vft {

class TriangulationTessellator : public Tessellator {
protected:
    std::shared_ptr<Font> _font{nullptr};
    unsigned int _fontSize{0};

    std::vector<glm::vec2> _vertices{};
    std::vector<CircularDLL<uint32_t>> _firstPolygon{};
    std::vector<CircularDLL<uint32_t>> _secondPolygon{};

public:
    TriangulationTessellator();
    ~TriangulationTessellator() = default;

    Glyph composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize = 0) override;

protected:
    std::set<float> _subdivideQuadraticBezier(const std::array<glm::vec2, 3> curve);
    void _subdivide(const std::array<glm::vec2, 3> curve, float t, float delta, std::set<float> &newVertices);
};

}  // namespace vft
