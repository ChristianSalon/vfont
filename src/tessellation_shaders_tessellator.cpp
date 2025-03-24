/**
 * @file tessellation_shaders_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "tessellation_shaders_tessellator.h"

namespace vft {

/**
 * @brief TessellationShadersTessellator constructor, initializes freetype outline decompose functions
 */
TessellationShadersTessellator::TessellationShadersTessellator() {
    this->_moveToFunc = [](const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        if (pThis->contourCount >= 2) {
            // Perform union of contours
            PolygonOperator polygonOperator{};
            polygonOperator.join(pThis->_currentGlyph.mesh.getVertices(), pThis->_firstPolygon, pThis->_secondPolygon);
            pThis->_currentGlyph.mesh.setVertices(polygonOperator.getVertices());
            pThis->_firstPolygon = polygonOperator.getPolygon();
            pThis->_secondPolygon = {CircularDLL<Edge>{}};

            pThis->vertexIndex = pThis->_currentGlyph.mesh.getVertexCount();
        } else if (pThis->contourCount == 1) {
            pThis->_firstPolygon = pThis->_secondPolygon;
            pThis->_secondPolygon = {CircularDLL<Edge>{}};
        }

        // Process contour starting vertex
        glm::vec2 vertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t vertexIndex = pThis->_getVertexIndex(vertex);
        if (vertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(vertex);
            pThis->vertexIndex++;
        }

        // Update glyph data
        pThis->contourStartVertexIndex = vertexIndex;
        pThis->lastVertex = vertex;
        pThis->lastVertexIndex = vertexIndex;
        pThis->contourCount++;

        return 0;
    };

    this->_lineToFunc = [](const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        // Process line end vertex
        glm::vec2 endVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t endVertexIndex = pThis->_getVertexIndex(endVertex);
        if (endVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(endVertex);
            pThis->vertexIndex++;
        }

        // Create line segment
        pThis->_currentGlyph.addLineSegment(Edge{pThis->lastVertexIndex, endVertexIndex});

        // Add edge to polygon
        pThis->_secondPolygon[0].insertLast(Edge{pThis->lastVertexIndex, endVertexIndex});

        // Update glyph data
        pThis->lastVertex = endVertex;
        pThis->lastVertexIndex = endVertexIndex;

        return 0;
    };

    this->_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        glm::vec2 startPoint = pThis->lastVertex;
        uint32_t startPointVertexIndex = pThis->lastVertexIndex;

        // Process curve control point
        glm::vec2 controlPoint{static_cast<float>(control->x), static_cast<float>(control->y)};
        uint32_t controlPointVertexIndex = pThis->_getVertexIndex(controlPoint);
        if (controlPointVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(controlPoint);
            pThis->vertexIndex++;
        }

        // Process curve end point
        glm::vec2 endPoint{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t endPointVertexIndex = pThis->_getVertexIndex(endPoint);
        if (endPointVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(endPoint);
            pThis->vertexIndex++;
        }

        // Create curve segment
        pThis->_currentGlyph.addCurveSegment(
            Curve{startPointVertexIndex, controlPointVertexIndex, endPointVertexIndex});

        if (pThis->_isOnLeftSide(startPoint, endPoint, controlPoint)) {
            // Add only edge from start point to end point
            pThis->_secondPolygon[0].insertLast(Edge{startPointVertexIndex, endPointVertexIndex});
        } else {
            // Add edge from start to control point and from control to end point
            pThis->_secondPolygon[0].insertLast(Edge{startPointVertexIndex, controlPointVertexIndex});
            pThis->_secondPolygon[0].insertLast(Edge{controlPointVertexIndex, endPointVertexIndex});
        }

        // Update glyph data
        pThis->lastVertex = endPoint;
        pThis->lastVertexIndex = endPointVertexIndex;

        return 0;
    };
}

/**
 * @brief Composes a glyph ready for rendering
 *
 * @param glyphId Id of glyph to compose
 * @param font Font of glyph
 * @param fontSize Font size of glyph
 */
Glyph TessellationShadersTessellator::composeGlyph(uint32_t glyphId,
                                                   std::shared_ptr<vft::Font> font,
                                                   unsigned int fontSize) {
    // Initialize polygons
    this->_firstPolygon = {CircularDLL<Edge>{}};
    this->_secondPolygon = {CircularDLL<Edge>{}};

    GlyphKey key{font->getFontFamily(), glyphId, 0};
    Glyph glyph = TessellationShadersTessellator::_composeGlyph(glyphId, font);

    std::vector<glm::vec2> vertices;
    std::vector<Edge> edges;
    std::vector<uint32_t> triangles;

    if (this->contourCount >= 1) {
        // Perform union of contours
        PolygonOperator polygonOperator{};
        polygonOperator.join(this->_currentGlyph.mesh.getVertices(), this->_firstPolygon, this->_secondPolygon);
        vertices = polygonOperator.getVertices();
        std::vector<CircularDLL<Edge>> polygon = polygonOperator.getPolygon();

        // Create edges for triangulation
        for (CircularDLL<Edge> &contour : polygon) {
            for (unsigned int i = 0; i < contour.size(); i++) {
                uint32_t startVertexIndex = contour.getAt(i)->value.first;
                uint32_t endVertexIndex = contour.getAt(i)->value.second;

                edges.push_back(Edge{startVertexIndex, endVertexIndex});
            }
        }

        // Remove duplicate vertices
        CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
            vertices, [](const glm::vec2 &p) { return p.x; }, [](const glm::vec2 &p) { return p.y; }, edges.begin(),
            edges.end(), [](const Edge &e) { return e.first; }, [](const Edge &e) { return e.second; },
            [](uint32_t i1, uint32_t i2) -> Edge {
                return Edge{i1, i2};
            });

        // Triangulation of inner triangles
        triangles = GlyphCompositor::triangulate(vertices, edges);
    }

    // Index buffer for curve segments
    std::vector<uint32_t> curveIndices;
    for (Curve curve : glyph.getCurveSegmentsIndices()) {
        curveIndices.push_back(curve.start);
        curveIndices.push_back(curve.control);
        curveIndices.push_back(curve.end);
    }

    // Set vertex and index buffer for composed glyph
    GlyphMesh mesh{vertices, {triangles, curveIndices}};
    glyph.mesh = mesh;

    return glyph;
}

/**
 * @brief Check whether point lies on the left side of line
 *
 * @param lineStartingPoint Line start
 * @param lineEndingPoint Line end
 * @param point Point to check
 *
 * @return True if point lies on the left of line
 */
bool TessellationShadersTessellator::_isOnLeftSide(const glm::vec2 &lineStartingPoint,
                                                   const glm::vec2 &lineEndingPoint,
                                                   const glm::vec2 &point) {
    float a = lineEndingPoint.y - lineStartingPoint.y;
    float b = lineStartingPoint.x - lineEndingPoint.x;
    float c = lineEndingPoint.x * lineStartingPoint.y - lineStartingPoint.x * lineEndingPoint.y;
    float d = a * point.x + b * point.y + c;

    return d < 0;
}

}  // namespace vft
