/**
 * @file tessellation_shaders_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "tessellation_shaders_tessellator.h"

namespace vft {

TessellationShadersTessellator::TessellationShadersTessellator() {
    this->_moveToFunc = [](const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        if (pThis->_currentGlyphData.contourCount >= 2) {
            // Perform union of contours
            PolygonOperator polygonOperator{};
            polygonOperator.join(pThis->_vertices, pThis->_firstPolygon, pThis->_secondPolygon);
            pThis->_firstPolygon = polygonOperator.getPolygon();
            pThis->_secondPolygon.clear();
        } else if (pThis->_currentGlyphData.contourCount == 1) {
            pThis->_firstPolygon = pThis->_secondPolygon;
            pThis->_secondPolygon.clear();
        }

        // Process contour starting vertex
        glm::vec2 newVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        pThis->_vertices.push_back(newVertex);
        uint32_t newVertexIndex = pThis->_currentGlyphData.vertexId;
        pThis->_currentGlyphData.vertexId++;

        // Add vertex to polygon
        pThis->_secondPolygon[0].insertLast(newVertexIndex);

        // Update glyph data
        pThis->_currentGlyphData.contourStartVertexId = newVertexIndex;
        pThis->_currentGlyphData.lastVertex = newVertex;
        pThis->_currentGlyphData.lastVertexIndex = newVertexIndex;
        pThis->_currentGlyphData.contourCount++;

        return 0;
    };

    this->_lineToFunc = [](const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        // Process line end vertex
        glm::vec2 endVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        pThis->_vertices.push_back(endVertex);
        uint32_t endVertexIndex = pThis->_currentGlyphData.vertexId;
        pThis->_currentGlyphData.vertexId++;

        // Create line segment
        pThis->_currentGlyph.addLineSegment(Edge{pThis->_currentGlyphData.lastVertexIndex, endVertexIndex});

        // Add vertex to polygon
        pThis->_secondPolygon[0].insertLast(endVertexIndex);

        // Update glyph data
        pThis->_currentGlyphData.lastVertex = endVertex;
        pThis->_currentGlyphData.lastVertexIndex = endVertexIndex;

        return 0;
    };

    this->_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        TessellationShadersTessellator *pThis = reinterpret_cast<TessellationShadersTessellator *>(user);

        glm::vec2 startPoint = pThis->_currentGlyphData.lastVertex;

        // Process curve control point
        glm::vec2 controlPoint{static_cast<float>(control->x), static_cast<float>(control->y)};
        pThis->_vertices.push_back(controlPoint);
        uint32_t controlPointVertexIndex = pThis->_currentGlyphData.vertexId;
        pThis->_currentGlyphData.vertexId++;

        // Process curve end point
        glm::vec2 endPoint{static_cast<float>(to->x), static_cast<float>(to->y)};
        pThis->_vertices.push_back(endPoint);
        uint32_t endPointVertexIndex = pThis->_currentGlyphData.vertexId;
        pThis->_currentGlyphData.vertexId++;

        // Create curve segment
        pThis->_currentGlyph.addCurveSegment(
            Curve{pThis->_currentGlyphData.lastVertexIndex, controlPointVertexIndex, endPointVertexIndex});

        if (pThis->_isOnLeftSide(startPoint, endPoint, controlPoint)) {
            // Add only end point to polygon
            pThis->_secondPolygon[0].insertLast(endPointVertexIndex);
        } else {
            // Add control point and end point to polygon
            pThis->_secondPolygon[0].insertLast(controlPointVertexIndex);
            pThis->_secondPolygon[0].insertLast(endPointVertexIndex);
        }

        // Update glyph data
        pThis->_currentGlyphData.lastVertex = endPoint;
        pThis->_currentGlyphData.lastVertexIndex = endPointVertexIndex;

        return 0;
    };
}

Glyph TessellationShadersTessellator::composeGlyph(uint32_t glyphId,
                                                   std::shared_ptr<vft::Font> font,
                                                   unsigned int fontSize) {
    GlyphKey key{font->getFontFamily(), glyphId, 0};
    Glyph glyph = TessellationShadersTessellator::_composeGlyph(glyphId, font);

    std::vector<glm::vec2> vertices;
    std::vector<Edge> edges;
    std::vector<uint32_t> triangles;

    if (this->_currentGlyphData.contourCount >= 1) {
        // Perform union of contours
        PolygonOperator polygonOperator{};
        polygonOperator.join(this->_vertices, this->_firstPolygon, this->_secondPolygon);
        vertices = polygonOperator.getVertices();
        std::vector<CircularDLL<uint32_t>> polygon = polygonOperator.getPolygon();

        // Create edges for triangulation
        for (CircularDLL<uint32_t> &contour : polygon) {
            for (unsigned int i = 0; i < contour.size(); i++) {
                uint32_t startVertexIndex = contour.getAt(i)->value;
                uint32_t endVertexIndex = contour.getAt(i)->next->value;

                edges.push_back(Edge{startVertexIndex, endVertexIndex});
            }
        }

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
