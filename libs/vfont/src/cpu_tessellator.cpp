/**
 * @file cpu_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "cpu_tessellator.h"

namespace vft {

CpuTessellator::CpuTessellator() {}

FT_Outline_MoveToFunc CpuTessellator::_moveToFunc = [](const FT_Vector *to, void *user) {
    CpuTessellator *pThis = reinterpret_cast<CpuTessellator *>(user);

    // Combine two contours into one
    if (CpuTessellator::_currentGlyphData.contourCount >= 2) {
        pThis->_combineContours();
    }

    // Start processing new contour
    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = Tessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(), newVertex);

    if (newVertexIndex == -1) {
        CpuTessellator::_currentGlyph.mesh.addVertex(newVertex);
        CpuTessellator::_currentGlyphData.contourStartVertexId = CpuTessellator::_currentGlyphData.vertexId;
        CpuTessellator::_currentGlyphData.vertexId++;
    } else {
        CpuTessellator::_currentGlyphData.contourStartVertexId = newVertexIndex;
    }

    CpuTessellator::_currentGlyphData.lastVertex = newVertex;
    CpuTessellator::_currentGlyphData.contourCount++;
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
FT_Outline_LineToFunc CpuTessellator::_lineToFunc = [](const FT_Vector *to, void *user) {
    CpuTessellator *pThis = reinterpret_cast<CpuTessellator *>(user);

    uint32_t lastVertexIndex = CpuTessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(),
                                                               CpuTessellator::_currentGlyphData.lastVertex);

    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = CpuTessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(), newVertex);
    if (newVertexIndex == -1) {
        CpuTessellator::_currentGlyph.mesh.addVertex(newVertex);
        newVertexIndex = CpuTessellator::_currentGlyphData.vertexId;
        CpuTessellator::_currentGlyphData.vertexId++;
    }

    vft::Edge lineSegment{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)};
    CpuTessellator::_currentGlyph.addLineSegment(lineSegment);
    pThis->_edges.push_back(lineSegment);

    CpuTessellator::_currentGlyphData.lastVertex = newVertex;
    return 0;
};

FT_Outline_ConicToFunc CpuTessellator::_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
    CpuTessellator *pThis = reinterpret_cast<CpuTessellator *>(user);

    uint32_t startPointIndex = CpuTessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(),
                                                               CpuTessellator::_currentGlyphData.lastVertex);
    glm::vec2 startPoint = CpuTessellator::_currentGlyph.mesh.getVertices().at(startPointIndex);

    glm::vec2 controlPoint = glm::vec2(static_cast<float>(control->x), static_cast<float>(control->y));
    int controlPointIndex =
        CpuTessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(), controlPoint);
    if (controlPointIndex == -1) {
        CpuTessellator::_currentGlyph.mesh.addVertex(controlPoint);
        controlPointIndex = CpuTessellator::_currentGlyphData.vertexId;
        CpuTessellator::_currentGlyphData.vertexId++;
    }

    glm::vec2 endPoint = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int endPointIndex = CpuTessellator::_getVertexIndex(CpuTessellator::_currentGlyph.mesh.getVertices(), endPoint);
    if (endPointIndex == -1) {
        CpuTessellator::_currentGlyph.mesh.addVertex(endPoint);
        endPointIndex = CpuTessellator::_currentGlyphData.vertexId;
        CpuTessellator::_currentGlyphData.vertexId++;
    }

    // Generate quadratic bezier curve segment for tessellation
    Curve curveSegment{startPointIndex, static_cast<uint32_t>(controlPointIndex), static_cast<uint32_t>(endPointIndex)};
    CpuTessellator::_currentGlyph.addCurveSegment(curveSegment);

    // Adaptive subdivision of quadratic bezier curve
    std::vector<glm::vec2> vertices = CpuTessellator::_currentGlyph.mesh.getVertices();
    std::array<glm::vec2, 3> curve = {
        pThis->_font->getScalingVector(pThis->_fontSize) * vertices.at(curveSegment.start),
        pThis->_font->getScalingVector(pThis->_fontSize) * vertices.at(curveSegment.control),
        pThis->_font->getScalingVector(pThis->_fontSize) * vertices.at(curveSegment.end)};
    std::set<float> newVertices = pThis->_subdivideQuadraticBezier(curve);
    if (newVertices.size() == 2) {
        pThis->_edges.push_back(vft::Edge{curveSegment.start, curveSegment.end});
    } else {
        int lastVertexIndex = CpuTessellator::_getVertexIndex(vertices, vertices.at(startPointIndex));

        for (float t : newVertices) {
            if (t == 0) {
                continue;
            }

            glm::vec2 newVertex{(1 - t) * (1 - t) * vertices.at(curveSegment.start) +
                                2 * (1 - t) * t * vertices.at(curveSegment.control) +
                                (t * t) * vertices.at(curveSegment.end)};
            int newVertexIndex = CpuTessellator::_getVertexIndex(vertices, newVertex);
            if (newVertexIndex < 0) {
                CpuTessellator::_currentGlyph.mesh.addVertex(newVertex);
                newVertexIndex = CpuTessellator::_currentGlyphData.vertexId;
                CpuTessellator::_currentGlyphData.vertexId++;
            }

            pThis->_edges.push_back(
                vft::Edge{static_cast<uint32_t>(lastVertexIndex), static_cast<uint32_t>(newVertexIndex)});
            lastVertexIndex = newVertexIndex;
        }
    }

    CpuTessellator::_currentGlyphData.lastVertex = endPoint;
    return 0;
};

Glyph CpuTessellator::composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize) {
    this->_font = font;
    this->_fontSize = fontSize;
    this->_edges.clear();

    GlyphKey key{font->getFontFamily(), glyphId, fontSize};
    Glyph glyph = this->_composeGlyph(glyphId, font);

    if (CpuTessellator::_currentGlyphData.contourCount >= 2) {
        std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
        this->_combineContours(vertices, this->_edges);
        glyph.mesh.setVertices(vertices);
    }

    std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
    std::vector<uint32_t> triangles = GlyphCompositor::triangulate(vertices, this->_edges);

    GlyphMesh mesh{vertices, {triangles}};
    glyph.mesh = mesh;

    return glyph;
}

int CpuTessellator::_combineContours(std::vector<glm::vec2> &vertices, std::vector<Edge> &edges) {
    std::set<uint32_t> intersections;
    CpuTessellator::_removeInverseEdges(edges, intersections);
    intersections = CpuTessellator::_resolveIntersectingEdges(vertices, edges);
    CpuTessellator::_removeInverseEdges(edges, intersections);
    CpuTessellator::_resolveSharedVertices(vertices, edges, intersections);
    CpuTessellator::_walkContours(vertices, edges, intersections);

    // Remove duplicate vertices
    CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
        vertices, [](const glm::vec2 &p) { return p.x; }, [](const glm::vec2 &p) { return p.y; }, edges.begin(),
        edges.end(), [](const vft::Edge &e) { return e.first; }, [](const vft::Edge &e) { return e.second; },
        [](uint32_t i1, uint32_t i2) -> vft::Edge {
            return vft::Edge{i1, i2};
        });

    return info.duplicates.size();
}

int CpuTessellator::_combineContours() {
    std::vector<glm::vec2> vertices = CpuTessellator::_currentGlyph.mesh.getVertices();
    std::vector<vft::Edge> edges = this->_edges;
    int duplicates = this->_combineContours(vertices, edges);

    CpuTessellator::_currentGlyphData.vertexId -= duplicates;
    CpuTessellator::_currentGlyph.mesh.setVertices(vertices);
    this->_edges = edges;

    return duplicates;
}

/**
 * @brief Computes vertices of a quadratic bezier curve with adaptive level of detail
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 */
std::set<float> CpuTessellator::_subdivideQuadraticBezier(const std::array<glm::vec2, 3> curve) {
    std::set<float> newVertices{0.f, 1.f};
    this->_subdivide(curve, 0.5f, 0.5f, newVertices);

    return newVertices;
}

/**
 * @brief Subdivides a quadratic bezier curve until there is no loss of quality
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 * @param t Parameter for the position on the bezier curve (t = <0, 1>)
 * @param delta Indicates distance between current point and computed points in the previous step
 * @param vertices Created vertices by dividing bezier curve into line segments
 */
void CpuTessellator::_subdivide(const std::array<glm::vec2, 3> curve,
                                float t,
                                float delta,
                                std::set<float> &newVertices) {
    // Get bezier curve points
    glm::vec2 startPoint = curve.at(0);
    glm::vec2 controlPoint = curve.at(1);
    glm::vec2 endPoint = curve.at(2);

    // Add current point
    glm::vec2 newVertex{(1 - t) * (1 - t) * startPoint + 2 * (1 - t) * t * controlPoint + (t * t) * endPoint};
    newVertices.insert(t);

    // Compute points around the current point with the given delta
    glm::vec2 left{(1 - (t - delta)) * (1 - (t - delta)) * startPoint +
                   2 * (1 - (t - delta)) * (t - delta) * controlPoint + (t - delta) * (t - delta) * endPoint};
    glm::vec2 right{(1 - (t + delta)) * (1 - (t + delta)) * startPoint +
                    2 * (1 - (t + delta)) * (t + delta) * controlPoint + (t + delta) * (t + delta) * endPoint};

    // Check if distance between curve(t - delta) and curve(t) is less than one pixel
    // or if points can be connected by a line without losing quality
    if (glm::length(newVertex - left) >= 1.f && newVertex.x != left.x && newVertex.y != left.y) {
        // The segment curve(t - delta) and curve(t) should be subdivided
        this->_subdivide(curve, t - delta / 2.f, delta / 2.f, newVertices);
    }

    // Check if distance between curve(t) and curve(t + delta) is less than one pixel
    // or if points can be connected by a line without losing quality
    if (glm::length(newVertex - right) >= 1.f && newVertex.x != right.x && newVertex.y != right.y) {
        // The segment curve(t) and curve(t + delta) should be subdivided
        this->_subdivide(curve, t + delta / 2.f, delta / 2.f, newVertices);
    }
}

FT_Outline_MoveToFunc CpuTessellator::_getMoveToFunc() {
    return CpuTessellator::_moveToFunc;
}

FT_Outline_LineToFunc CpuTessellator::_getLineToFunc() {
    return CpuTessellator::_lineToFunc;
}

FT_Outline_ConicToFunc CpuTessellator::_getConicToFunc() {
    return CpuTessellator::_conicToFunc;
}

}  // namespace vft
