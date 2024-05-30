/**
 * @file glyph_compositor.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <memory>
#include <map>
#include <set>
#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/vec2.hpp>

#include "glyph.h"
#include "text_renderer_utils.h"
#include "font.h"

namespace vft {

/**
 * @class GlyphCompositor
 *
 * @brief Composes a triangulated glyph from glyph outlines
 */
class GlyphCompositor {

protected:

    static FT_Outline_MoveToFunc _moveToFunc;
    static FT_Outline_LineToFunc _lineToFunc;
    static FT_Outline_ConicToFunc _conicToFunc;
    static FT_Outline_CubicToFunc _cubicToFunc;

    Glyph _currentGlyph;                                /**< Glyph that is currently being composed */
    vft::ComposedGlyphData _currentGlyphData;           /**< Data of glyph that is currently being composed */

public:

    void compose(uint32_t codePoint, std::shared_ptr<Font> font);

protected:

    void _composeGlyph(uint32_t codePoint, std::shared_ptr<Font> font);
    void _triangulate();

    int _getVertexIndex(const std::vector<glm::vec2> &vertices, glm::vec2 vertex);
    void _detailBezier(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint);
    void _subdivide(
        glm::vec2 startPoint,
        glm::vec2 controlPoint,
        glm::vec2 endPoint,
        float t,
        float delta,
        std::map<float, glm::vec2> &vertices);

    double _determinant(double a, double b, double c, double d);
    bool _isPointOnLineSegment(double x1, double y1, double x2, double y2, double x, double y);
    bool _intersect(
        const std::vector<glm::vec2> &vertices,
        vft::Edge first,
        vft::Edge second,
        glm::vec2 &intersection);
    std::set<uint32_t> _resolveIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges);

    void _removeDuplicateEdges(std::vector<vft::Edge> &edges);
    void _removeInverseEdges(std::vector<vft::Edge> &edges, std::set<uint32_t> &intersections);
    void _resolveSharedVertices(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges, std::set<uint32_t> &intersections);
    //void _walkContours(const std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges);
    void _walkContours(const std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges, std::set<uint32_t> intersections);
    std::vector<uint32_t> _findAllEdgesContainingVertex(uint32_t vertexId, const std::vector<vft::Edge> &edges);
    void _updateVisitedEdgesUntilNextIntersection(std::vector<vft::Edge> &edges, std::set<uint32_t> &visited, uint32_t startEdgeIndex);
    void _updateVisitedEdgesUntilNextIntersection(std::vector<vft::Edge> &edges, std::set<uint32_t> &visited, uint32_t startEdgeIndex, uint32_t endVertexId);
    void _updateVisitedEdges(std::vector<vft::Edge> &edges, std::set<uint32_t> &visited, uint32_t startEdgeIndex, uint32_t endVertexId);

};

}
