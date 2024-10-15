/**
 * @file tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <map>
#include <set>
#include <cstdint>
#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <glm/vec2.hpp>

#include "font.h"
#include "glyph.h"
#include "glyph_cache.h"
#include "text_renderer_utils.h"

namespace vft {

class Tessellator {

protected:

    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;
    static FT_Outline_CubicToFunc _cubicToFunc;

    static Glyph _currentGlyph;                                /**< Glyph that is currently being composed */
    static vft::ComposedGlyphData _currentGlyphData;           /**< Data of glyph that is currently being composed */

    GlyphCache &_cache;

public:

    Tessellator(GlyphCache &cache);
    ~Tessellator() = default;

    virtual Glyph composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) = 0;

protected:

    Glyph _composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font);

    virtual FT_Outline_MoveToFunc _getMoveToFunc();
    virtual FT_Outline_LineToFunc _getLineToFunc();
    virtual FT_Outline_ConicToFunc _getConicToFunc();
    virtual FT_Outline_CubicToFunc _getCubicToFunc();

    static bool _isOnRightSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point);
    static int _getVertexIndex(const std::vector<glm::vec2>& vertices, glm::vec2 vertex);

    static std::set<uint32_t> _resolveIntersectingEdges(std::vector<glm::vec2>& vertices, std::vector<vft::Edge>& edges);
    static double _determinant(double a, double b, double c, double d);
    static bool _intersect(
        const std::vector<glm::vec2>& vertices,
        vft::Edge first,
        vft::Edge second,
        glm::vec2& intersection);
    static void _removeDuplicateEdges(std::vector<vft::Edge>& edges);
    static void _removeInverseEdges(std::vector<vft::Edge>& edges, std::set<uint32_t>& intersections);
    static void _resolveSharedVertices(std::vector<glm::vec2>& vertices, std::vector<vft::Edge>& edges, std::set<uint32_t>& intersections);
    static void _walkContours(const std::vector<glm::vec2>& vertices, std::vector<vft::Edge>& edges, std::set<uint32_t> intersections);
    static std::vector<uint32_t> _getEdgesStartingAt(uint32_t vertexId, const std::vector<vft::Edge>& edges);
    static void _visitEdgesUntilIntersection(std::vector<vft::Edge>& edges, std::set<uint32_t>& visited, uint32_t startEdgeIndex);
    static bool _isInPositiveRegion(glm::vec2 startVertex, glm::vec2 endVertex, glm::vec2 vertex);

};

}
