/**
 * @file triangulation_tessellator.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "triangulation_tessellator.h"

namespace vft {

/**
 * @brief TriangulationTessellator constructor, initializes freetype outline decompose functions
 */
TriangulationTessellator::TriangulationTessellator() {
    this->_moveToFunc = [](const FT_Vector *to, void *user) {
        TriangulationTessellator *pThis = reinterpret_cast<TriangulationTessellator *>(user);

        // Assign orientation of contour to second polygon
        pThis->_secondPolygon[0].orientation = pThis->area >= 0 ? Outline::Orientation::CCW : Outline::Orientation::CW;

        if (pThis->contourCount >= 2) {
            // Perform union of contours
            PolygonOperator polygonOperator{};
            polygonOperator.join(pThis->_currentGlyph.mesh.getVertices(), pThis->_firstPolygon, pThis->_secondPolygon);
            pThis->_currentGlyph.mesh.setVertices(polygonOperator.getVertices());
            pThis->_firstPolygon = polygonOperator.getPolygon();
            pThis->_secondPolygon = {Outline{}};

            pThis->vertexIndex = pThis->_currentGlyph.mesh.getVertexCount();
        } else if (pThis->contourCount == 1) {
            pThis->_firstPolygon = pThis->_secondPolygon;
            pThis->_secondPolygon = {Outline{}};
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
        pThis->area = 0;

        return 0;
    };

    this->_lineToFunc = [](const FT_Vector *to, void *user) {
        TriangulationTessellator *pThis = reinterpret_cast<TriangulationTessellator *>(user);

        // Process line end vertex
        glm::vec2 endVertex{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t endVertexIndex = pThis->_getVertexIndex(endVertex);
        if (endVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(endVertex);
            pThis->vertexIndex++;
        }

        Edge edge{pThis->lastVertexIndex, endVertexIndex};
        if (edge.first != edge.second) {
            // Create line segment
            pThis->_currentGlyph.addLineSegment(edge);
            // Add edge to polygon
            pThis->_secondPolygon[0].edges.insertLast(edge);
            // Update signed area of polygon
            pThis->area += pThis->lastVertex.x * endVertex.y - endVertex.x * pThis->lastVertex.y;
        }

        // Update glyph data
        pThis->lastVertex = endVertex;
        pThis->lastVertexIndex = endVertexIndex;

        return 0;
    };

    this->_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
        TriangulationTessellator *pThis = reinterpret_cast<TriangulationTessellator *>(user);

        glm::vec2 startPoint = pThis->lastVertex;

        // Process curve control vertex
        glm::vec2 controlPoint{static_cast<float>(control->x), static_cast<float>(control->y)};
        uint32_t controlPointVertexIndex = pThis->_getVertexIndex(controlPoint);
        if (controlPointVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(controlPoint);
            pThis->vertexIndex++;
        }

        // Process curve end vertex
        glm::vec2 endPoint{static_cast<float>(to->x), static_cast<float>(to->y)};
        uint32_t endPointVertexIndex = pThis->_getVertexIndex(endPoint);
        if (endPointVertexIndex == pThis->vertexIndex) {
            pThis->_currentGlyph.mesh.addVertex(endPoint);
            pThis->vertexIndex++;
        }

        // Create curve segment
        pThis->_currentGlyph.addCurveSegment(
            Curve{pThis->lastVertexIndex, controlPointVertexIndex, endPointVertexIndex});

        // Adaptive subdivision of quadratic bezier curve
        std::array<glm::vec2, 3> curve{pThis->_font->getScalingVector(pThis->_fontSize) * startPoint,
                                       pThis->_font->getScalingVector(pThis->_fontSize) * controlPoint,
                                       pThis->_font->getScalingVector(pThis->_fontSize) * endPoint};
        std::set<float> newVertices = pThis->_subdivideQuadraticBezier(curve);

        glm::vec2 lastVertex = pThis->lastVertex;
        uint32_t lastVertexIndex = pThis->lastVertexIndex;
        for (float t : newVertices) {
            if (t == 0) {
                continue;
            }

            glm::vec2 newVertex{(1 - t) * (1 - t) * startPoint + 2 * (1 - t) * t * controlPoint + (t * t) * endPoint};
            uint32_t newVertexIndex = pThis->_getVertexIndex(newVertex);
            if (newVertexIndex == pThis->vertexIndex) {
                pThis->_currentGlyph.mesh.addVertex(newVertex);
                pThis->vertexIndex++;
            }

            Edge edge{lastVertexIndex, newVertexIndex};
            if (edge.first != edge.second) {
                // Add edge to polygon
                pThis->_secondPolygon[0].edges.insertLast(edge);
                // Update signed area of contour
                pThis->area += lastVertex.x * newVertex.y - newVertex.x * lastVertex.y;
            }

            lastVertex = newVertex;
            lastVertexIndex = newVertexIndex;
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
Glyph TriangulationTessellator::composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize) {
    this->_font = font;
    this->_fontSize = fontSize;

    // Initialize polygons
    this->_firstPolygon = {Outline{}};
    this->_secondPolygon = {Outline{}};

    GlyphKey key{font->getFontFamily(), glyphId, fontSize};
    Glyph glyph = this->_composeGlyph(glyphId, font);

    std::vector<glm::vec2> vertices;
    std::vector<Edge> edges;
    std::vector<uint32_t> triangles;

    if (this->contourCount >= 1) {
        // Assign orientation of contour to second polygon
        this->_secondPolygon[0].orientation = this->area >= 0 ? Outline::Orientation::CCW : Outline::Orientation::CW;

        // Perform union of contours
        PolygonOperator polygonOperator{};
        polygonOperator.join(this->_currentGlyph.mesh.getVertices(), this->_firstPolygon, this->_secondPolygon);
        vertices = polygonOperator.getVertices();
        std::vector<Outline> polygon = polygonOperator.getPolygon();

        // Create edges for triangulation
        for (Outline &outline : polygon) {
            for (unsigned int i = 0; i < outline.edges.size(); i++) {
                uint32_t startVertexIndex = outline.edges.getAt(i)->value.first;
                uint32_t endVertexIndex = outline.edges.getAt(i)->value.second;

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

        // Triangulation
        triangles = GlyphCompositor::triangulate(vertices, edges);
    }

    GlyphMesh mesh{vertices, {triangles}};
    glyph.mesh = mesh;

    // Cleanup
    this->_firstPolygon.clear();
    this->_secondPolygon.clear();

    return glyph;
}

/**
 * @brief Computes vertices of a quadratic bezier curve with adaptive level of detail
 *
 * @param curve Bezier curve start, control and end points
 */
std::set<float> TriangulationTessellator::_subdivideQuadraticBezier(const std::array<glm::vec2, 3> curve) {
    std::set<float> newVertices{0.f, 1.f};
    this->_subdivide(curve, 0.5f, 0.5f, newVertices);

    return newVertices;
}

/**
 * @brief Subdivides a quadratic bezier curve until there is no loss of quality
 *
 * @param curve Bezier curve start, control and end points
 * @param t Parameter for the position on the bezier curve (t = <0, 1>)
 * @param delta Indicates distance between current point and computed points in the previous step
 * @param newVertices T parameters created by dividing bezier curve into line segments
 */
void TriangulationTessellator::_subdivide(const std::array<glm::vec2, 3> curve,
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

}  // namespace vft
