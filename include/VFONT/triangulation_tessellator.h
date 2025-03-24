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

#include <glm/glm.hpp>

#include "circular_dll.h"
#include "curve.h"
#include "edge.h"
#include "font.h"
#include "glyph.h"
#include "glyph_cache.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "polygon_operator.h"
#include "tessellator.h"

namespace vft {

/**
 * @brief Composes glyphs that are fully triangulated on the cpu using the constrained delaunay triangulation algorithm
 */
class TriangulationTessellator : public Tessellator {
protected:
    std::shared_ptr<Font> _font{nullptr}; /**< Font of current glyph */
    unsigned int _fontSize{0};            /**< Font size of current glyph */

    std::vector<CircularDLL<Edge>> _firstPolygon{};  /**< Polygon containing processed glyph contours */
    std::vector<CircularDLL<Edge>> _secondPolygon{}; /**< Polygon containing current contour */

public:
    TriangulationTessellator();
    ~TriangulationTessellator() = default;

    Glyph composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize = 0) override;

protected:
    std::set<float> _subdivideQuadraticBezier(const std::array<glm::vec2, 3> curve);
    void _subdivide(const std::array<glm::vec2, 3> curve, float t, float delta, std::set<float> &newVertices);
};

}  // namespace vft
