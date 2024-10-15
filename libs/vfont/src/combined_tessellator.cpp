/**
 * @file combined_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "combined_tessellator.h"

namespace vft {

FT_Outline_MoveToFunc CombinedTessellator::_moveToFunc = [](const FT_Vector *to, void *user) {
    CombinedTessellator *pThis = reinterpret_cast<CombinedTessellator *>(user);

    // Combine two contours into one
    if (CombinedTessellator::_currentGlyphData.contourCount >= 2) {
        pThis->_combineContours();
    }

    // Start processing new contour
    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = Tessellator::_getVertexIndex(CombinedTessellator::_currentGlyph.mesh.getVertices(), newVertex);

    if (newVertexIndex == -1) {
        CombinedTessellator::_currentGlyph.mesh.addVertex(newVertex);
        CombinedTessellator::_currentGlyphData.contourStartVertexId = CombinedTessellator::_currentGlyphData.vertexId;
        CombinedTessellator::_currentGlyphData.vertexId++;
    } else {
        CombinedTessellator::_currentGlyphData.contourStartVertexId = newVertexIndex;
    }

    CombinedTessellator::_currentGlyphData.lastVertex = newVertex;
    CombinedTessellator::_currentGlyphData.contourCount++;
    return 0;
};

/**
 * @brief Freetype outline decomposition line_to function.
 * Used when between two points is a line segment.
 *
 * @param to Line segment ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_LineToFunc CombinedTessellator::_lineToFunc = [](const FT_Vector *to, void *user) {
    CombinedTessellator *pThis = reinterpret_cast<CombinedTessellator *>(user);

    uint32_t lastVertexIndex = CombinedTessellator::_getVertexIndex(
        CombinedTessellator::_currentGlyph.mesh.getVertices(), CombinedTessellator::_currentGlyphData.lastVertex);

    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex =
        CombinedTessellator::_getVertexIndex(CombinedTessellator::_currentGlyph.mesh.getVertices(), newVertex);
    if (newVertexIndex == -1) {
        CombinedTessellator::_currentGlyph.mesh.addVertex(newVertex);
        newVertexIndex = CombinedTessellator::_currentGlyphData.vertexId;
        CombinedTessellator::_currentGlyphData.vertexId++;
    }

    vft::Edge lineSegment{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)};
    CombinedTessellator::_currentGlyph.addLineSegment(lineSegment);
    pThis->_edges.push_back(lineSegment);

    CombinedTessellator::_currentGlyphData.lastVertex = newVertex;
    return 0;
};

FT_Outline_ConicToFunc CombinedTessellator::_conicToFunc = [](const FT_Vector *control,
                                                              const FT_Vector *to,
                                                              void *user) {
    CombinedTessellator *pThis = reinterpret_cast<CombinedTessellator *>(user);

    uint32_t startPointIndex = CombinedTessellator::_getVertexIndex(
        CombinedTessellator::_currentGlyph.mesh.getVertices(), CombinedTessellator::_currentGlyphData.lastVertex);
    glm::vec2 startPoint = CombinedTessellator::_currentGlyph.mesh.getVertices().at(startPointIndex);

    glm::vec2 controlPoint = glm::vec2(static_cast<float>(control->x), static_cast<float>(control->y));
    int controlPointIndex =
        CombinedTessellator::_getVertexIndex(CombinedTessellator::_currentGlyph.mesh.getVertices(), controlPoint);
    if (controlPointIndex == -1) {
        CombinedTessellator::_currentGlyph.mesh.addVertex(controlPoint);
        controlPointIndex = CombinedTessellator::_currentGlyphData.vertexId;
        CombinedTessellator::_currentGlyphData.vertexId++;
    }

    glm::vec2 endPoint = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int endPointIndex =
        CombinedTessellator::_getVertexIndex(CombinedTessellator::_currentGlyph.mesh.getVertices(), endPoint);
    if (endPointIndex == -1) {
        CombinedTessellator::_currentGlyph.mesh.addVertex(endPoint);
        endPointIndex = CombinedTessellator::_currentGlyphData.vertexId;
        CombinedTessellator::_currentGlyphData.vertexId++;
    }

    // Generate quadratic bezier curve segment for tessellation
    Curve curveSegment{startPointIndex, static_cast<uint32_t>(controlPointIndex), static_cast<uint32_t>(endPointIndex)};
    CombinedTessellator::_currentGlyph.addCurveSegment(curveSegment);

    std::vector<glm::vec2> vertices = CombinedTessellator::_currentGlyph.mesh.getVertices();
    if (CombinedTessellator::_isOnRightSide(vertices.at(curveSegment.start), vertices.at(curveSegment.end),
                                            vertices.at(curveSegment.control))) {
        pThis->_edges.push_back({curveSegment.start, curveSegment.control});
        pThis->_edges.push_back({curveSegment.control, curveSegment.end});
    } else {
        pThis->_edges.push_back({curveSegment.start, curveSegment.end});
    }

    CombinedTessellator::_currentGlyphData.lastVertex = endPoint;
    return 0;
};

CombinedTessellator::CombinedTessellator(GlyphCache &cache) : Tessellator{cache} {}

Glyph CombinedTessellator::composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) {
    GlyphKey key{font->getFontFamily(), codePoint};
    if (this->_cache.exists(key)) {
        return this->_cache.getGlyph(key);
    }

    this->_edges.clear();
    Glyph glyph = CombinedTessellator::_composeGlyph(codePoint, font);
    if (CombinedTessellator::_currentGlyphData.contourCount >= 2) {
        std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
        this->_combineContours(vertices, this->_edges);
        glyph.mesh.setVertices(vertices);
    }

    std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
    std::vector<uint32_t> triangles = GlyphCompositor::triangulate(vertices, this->_edges);

    std::vector<uint32_t> curveIndices;
    for (vft::Curve curve : glyph.getCurveSegmentsIndices()) {
        curveIndices.push_back(curve.start);
        curveIndices.push_back(curve.control);
        curveIndices.push_back(curve.end);
    }

    GlyphMesh mesh{vertices, {triangles, curveIndices}};
    glyph.mesh = mesh;
    this->_cache.setGlyph(key, glyph);

    return glyph;
}

int CombinedTessellator::_combineContours(std::vector<glm::vec2> &vertices, std::vector<Edge> &edges) {
    std::set<uint32_t> intersections;
    CombinedTessellator::_removeInverseEdges(edges, intersections);
    intersections = CombinedTessellator::_resolveIntersectingEdges(vertices, edges);
    CombinedTessellator::_removeInverseEdges(edges, intersections);
    CombinedTessellator::_resolveSharedVertices(vertices, edges, intersections);
    CombinedTessellator::_walkContours(vertices, edges, intersections);

    // Remove duplicate vertices
    CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
        vertices, [](const glm::vec2 &p) { return p.x; }, [](const glm::vec2 &p) { return p.y; }, edges.begin(),
        edges.end(), [](const vft::Edge &e) { return e.first; }, [](const vft::Edge &e) { return e.second; },
        [](uint32_t i1, uint32_t i2) -> vft::Edge {
            return vft::Edge{i1, i2};
        });

    return info.duplicates.size();
}

int CombinedTessellator::_combineContours() {
    std::vector<glm::vec2> vertices = CombinedTessellator::_currentGlyph.mesh.getVertices();
    std::vector<vft::Edge> edges = this->_edges;
    int duplicates = this->_combineContours(vertices, edges);

    CombinedTessellator::_currentGlyphData.vertexId -= duplicates;
    CombinedTessellator::_currentGlyph.mesh.setVertices(vertices);
    this->_edges = edges;

    return duplicates;
}

FT_Outline_MoveToFunc CombinedTessellator::_getMoveToFunc() {
    return CombinedTessellator::_moveToFunc;
}

FT_Outline_LineToFunc CombinedTessellator::_getLineToFunc() {
    return CombinedTessellator::_lineToFunc;
}

FT_Outline_ConicToFunc CombinedTessellator::_getConicToFunc() {
    return CombinedTessellator::_conicToFunc;
}

}  // namespace vft
