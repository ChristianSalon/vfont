/**
 * @file tessellation_shaders_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "tessellation_shaders_tessellator.h"

namespace vft {

FT_Outline_MoveToFunc TessellationShadersTessellator::_moveToFunc = [](const FT_Vector *to, void *user) {
    TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

    // Combine two contours into one
    if (TessellationShadersTessellator::_currentGlyphData.contourCount >= 2) {
        pThis->_combineContours();
    }

    // Start processing new contour
    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex =
        Tessellator::_getVertexIndex(TessellationShadersTessellator::_currentGlyph.mesh.getVertices(), newVertex);

    if (newVertexIndex == -1) {
        TessellationShadersTessellator::_currentGlyph.mesh.addVertex(newVertex);
        TessellationShadersTessellator::_currentGlyphData.contourStartVertexId =
            TessellationShadersTessellator::_currentGlyphData.vertexId;
        TessellationShadersTessellator::_currentGlyphData.vertexId++;
    } else {
        TessellationShadersTessellator::_currentGlyphData.contourStartVertexId = newVertexIndex;
    }

    TessellationShadersTessellator::_currentGlyphData.lastVertex = newVertex;
    TessellationShadersTessellator::_currentGlyphData.contourCount++;
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
FT_Outline_LineToFunc TessellationShadersTessellator::_lineToFunc = [](const FT_Vector *to, void *user) {
    TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

    uint32_t lastVertexIndex = TessellationShadersTessellator::_getVertexIndex(
        TessellationShadersTessellator::_currentGlyph.mesh.getVertices(),
        TessellationShadersTessellator::_currentGlyphData.lastVertex);

    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = TessellationShadersTessellator::_getVertexIndex(
        TessellationShadersTessellator::_currentGlyph.mesh.getVertices(), newVertex);
    if (newVertexIndex == -1) {
        TessellationShadersTessellator::_currentGlyph.mesh.addVertex(newVertex);
        newVertexIndex = TessellationShadersTessellator::_currentGlyphData.vertexId;
        TessellationShadersTessellator::_currentGlyphData.vertexId++;
    }

    vft::Edge lineSegment{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)};
    TessellationShadersTessellator::_currentGlyph.addLineSegment(lineSegment);
    pThis->_edges.push_back(lineSegment);

    TessellationShadersTessellator::_currentGlyphData.lastVertex = newVertex;
    return 0;
};

FT_Outline_ConicToFunc TessellationShadersTessellator::_conicToFunc = [](const FT_Vector *control,
                                                                         const FT_Vector *to,
                                                                         void *user) {
    TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

    uint32_t startPointIndex = TessellationShadersTessellator::_getVertexIndex(
        TessellationShadersTessellator::_currentGlyph.mesh.getVertices(),
        TessellationShadersTessellator::_currentGlyphData.lastVertex);
    glm::vec2 startPoint = TessellationShadersTessellator::_currentGlyph.mesh.getVertices().at(startPointIndex);

    glm::vec2 controlPoint = glm::vec2(static_cast<float>(control->x), static_cast<float>(control->y));
    int controlPointIndex = TessellationShadersTessellator::_getVertexIndex(
        TessellationShadersTessellator::_currentGlyph.mesh.getVertices(), controlPoint);
    if (controlPointIndex == -1) {
        TessellationShadersTessellator::_currentGlyph.mesh.addVertex(controlPoint);
        controlPointIndex = TessellationShadersTessellator::_currentGlyphData.vertexId;
        TessellationShadersTessellator::_currentGlyphData.vertexId++;
    }

    glm::vec2 endPoint = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int endPointIndex = TessellationShadersTessellator::_getVertexIndex(
        TessellationShadersTessellator::_currentGlyph.mesh.getVertices(), endPoint);
    if (endPointIndex == -1) {
        TessellationShadersTessellator::_currentGlyph.mesh.addVertex(endPoint);
        endPointIndex = TessellationShadersTessellator::_currentGlyphData.vertexId;
        TessellationShadersTessellator::_currentGlyphData.vertexId++;
    }

    // Generate quadratic bezier curve segment for tessellation
    Curve curveSegment{startPointIndex, static_cast<uint32_t>(controlPointIndex), static_cast<uint32_t>(endPointIndex)};
    TessellationShadersTessellator::_currentGlyph.addCurveSegment(curveSegment);

    std::vector<glm::vec2> vertices = TessellationShadersTessellator::_currentGlyph.mesh.getVertices();
    if (TessellationShadersTessellator::_isOnRightSide(vertices.at(curveSegment.start), vertices.at(curveSegment.end),
                                                       vertices.at(curveSegment.control))) {
        pThis->_edges.push_back({curveSegment.start, curveSegment.control});
        pThis->_edges.push_back({curveSegment.control, curveSegment.end});
    } else {
        pThis->_edges.push_back({curveSegment.start, curveSegment.end});
    }

    TessellationShadersTessellator::_currentGlyphData.lastVertex = endPoint;
    return 0;
};

TessellationShadersTessellator::TessellationShadersTessellator() {}

Glyph TessellationShadersTessellator::composeGlyph(uint32_t glyphId,
                                                   std::shared_ptr<vft::Font> font,
                                                   unsigned int fontSize) {
    this->_edges.clear();

    GlyphKey key{font->getFontFamily(), glyphId, 0};
    Glyph glyph = TessellationShadersTessellator::_composeGlyph(glyphId, font);

    if (TessellationShadersTessellator::_currentGlyphData.contourCount >= 2) {
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

    return glyph;
}

int TessellationShadersTessellator::_combineContours(std::vector<glm::vec2> &vertices, std::vector<Edge> &edges) {
    std::set<uint32_t> intersections;
    TessellationShadersTessellator::_removeInverseEdges(edges, intersections);
    intersections = TessellationShadersTessellator::_resolveIntersectingEdges(vertices, edges);
    TessellationShadersTessellator::_removeInverseEdges(edges, intersections);
    TessellationShadersTessellator::_resolveSharedVertices(vertices, edges, intersections);
    TessellationShadersTessellator::_walkContours(vertices, edges, intersections);

    // Remove duplicate vertices
    CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
        vertices, [](const glm::vec2 &p) { return p.x; }, [](const glm::vec2 &p) { return p.y; }, edges.begin(),
        edges.end(), [](const vft::Edge &e) { return e.first; }, [](const vft::Edge &e) { return e.second; },
        [](uint32_t i1, uint32_t i2) -> vft::Edge {
            return vft::Edge{i1, i2};
        });

    return info.duplicates.size();
}

int TessellationShadersTessellator::_combineContours() {
    std::vector<glm::vec2> vertices = TessellationShadersTessellator::_currentGlyph.mesh.getVertices();
    std::vector<vft::Edge> edges = this->_edges;
    int duplicates = this->_combineContours(vertices, edges);

    TessellationShadersTessellator::_currentGlyphData.vertexId -= duplicates;
    TessellationShadersTessellator::_currentGlyph.mesh.setVertices(vertices);
    this->_edges = edges;

    return duplicates;
}

FT_Outline_MoveToFunc TessellationShadersTessellator::_getMoveToFunc() {
    return TessellationShadersTessellator::_moveToFunc;
}

FT_Outline_LineToFunc TessellationShadersTessellator::_getLineToFunc() {
    return TessellationShadersTessellator::_lineToFunc;
}

FT_Outline_ConicToFunc TessellationShadersTessellator::_getConicToFunc() {
    return TessellationShadersTessellator::_conicToFunc;
}

}  // namespace vft
