/**
 * @file glyph_compositor.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include "CDT.h"

#include "glyph_compositor.h"

namespace vft {

/**
 * @brief Freetype outline decomposition move_to function.
 * Used when new contour is detected.
 *
 * @param to Contour starting vertex
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_MoveToFunc GlyphCompositor::_moveToFunc = [](const FT_Vector *to, void *user) {
    GlyphCompositor *pThis = reinterpret_cast<GlyphCompositor *>(user);

    // Makes the contour closed
    if(pThis->_currentGlyphData.contourCount != 0) {
        pThis->_currentGlyph.updateEdge(
            pThis->_currentGlyph.getEdgeCount() - 1,
            vft::Edge{pThis->_currentGlyph.getEdges().at(pThis->_currentGlyph.getEdgeCount() - 1).first, pThis->_currentGlyphData.contourStartVertexId});
    }

    // Combine two contours into one
    if(pThis->_currentGlyphData.contourCount >= 2) {
        std::vector<glm::vec2> vertices = pThis->_currentGlyph.getVertices();
        std::vector<vft::Edge> edges = pThis->_currentGlyph.getEdges();

        std::set<uint32_t> intersections;
        pThis->_removeInverseEdges(edges, intersections);
        intersections = pThis->_resolveIntersectingEdges(vertices, edges);
        pThis->_removeInverseEdges(edges, intersections);
        pThis->_resolveSharedVertices(vertices, edges, intersections);
        pThis->_walkContours(vertices, edges, intersections);

        // Remove duplicate vertices
        CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
            vertices,
            [](const glm::vec2 &p) { return p.x; },
            [](const glm::vec2 &p) { return p.y; },
            edges.begin(),
            edges.end(),
            [](const vft::Edge &e) { return e.first; },
            [](const vft::Edge &e) { return e.second; },
            [](uint32_t i1, uint32_t i2) -> vft::Edge { return vft::Edge{i1, i2}; });
        pThis->_currentGlyphData.vertexId -= info.duplicates.size();

        pThis->_currentGlyph.setVertices(vertices);
        pThis->_currentGlyph.setEdges(edges);
    }

    // Start processing new contour
    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = pThis->_getVertexIndex(pThis->_currentGlyph.getVertices(), newVertex);

    if(newVertexIndex == -1) {
        pThis->_currentGlyph.addVertex(newVertex);
        pThis->_currentGlyphData.contourStartVertexId = pThis->_currentGlyphData.vertexId;
        pThis->_currentGlyphData.vertexId++;
    }
    else {
        pThis->_currentGlyphData.contourStartVertexId = newVertexIndex;
    }

    pThis->_currentGlyphData.lastVertex = newVertex;
    pThis->_currentGlyphData.contourCount++;

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
FT_Outline_LineToFunc GlyphCompositor::_lineToFunc = [](const FT_Vector *to, void *user) {
    GlyphCompositor *pThis = reinterpret_cast<GlyphCompositor *>(user);

    uint32_t lastVertexIndex = pThis->_getVertexIndex(pThis->_currentGlyph.getVertices(), pThis->_currentGlyphData.lastVertex);

    glm::vec2 newVertex = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));
    int newVertexIndex = pThis->_getVertexIndex(pThis->_currentGlyph.getVertices(), newVertex);

    if(newVertexIndex == -1) {
        pThis->_currentGlyph.addVertex(newVertex);
        pThis->_currentGlyph.addEdge(vft::Edge{lastVertexIndex, pThis->_currentGlyphData.vertexId});
        pThis->_currentGlyphData.vertexId++;
    }
    else {
        pThis->_currentGlyph.addEdge(vft::Edge{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)});
    }

    pThis->_currentGlyphData.lastVertex = newVertex;

    return 0;
};

/**
 * @brief Freetype outline decomposition conic_to function.
 * Used when there is a quadratic bezier curve.
 *
 * @param control Bezier curve control point
 * @param to Bezier curve ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_ConicToFunc GlyphCompositor::_conicToFunc = [](const FT_Vector *control, const FT_Vector *to, void *user) {
    GlyphCompositor *pThis = reinterpret_cast<GlyphCompositor *>(user);

    glm::vec2 controlPoint = glm::vec2(static_cast<float>(control->x), static_cast<float>(control->y));
    glm::vec2 endPoint = glm::vec2(static_cast<float>(to->x), static_cast<float>(to->y));

    pThis->_detailBezier(pThis->_currentGlyphData.lastVertex, controlPoint, endPoint);
    pThis->_currentGlyphData.lastVertex = endPoint;

    return 0;
};

/**
 * @brief Freetype outline decomposition cubic_to function.
 * Used when there is a cubic bezier curve.
 *
 * @param control1 First bezier curve control point
 * @param control2 Second bezier curve control point
 * @param to Bezier curve ending point
 * @param user User defined data
 *
 * @return Exit code
 */
FT_Outline_CubicToFunc GlyphCompositor::_cubicToFunc = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
    throw std::runtime_error("Fonts with cubic bezier curves are not supported");
    return 0;
};

int GlyphCompositor::_getVertexIndex(const std::vector<glm::vec2> &vertices, glm::vec2 vertex) {
    for(int i = 0; i < vertices.size(); i++) {
        if(glm::distance(vertex, vertices.at(i)) <= 1.f) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Computes vertices of a quadratic bezier curve with adaptive level of detail
 *
 * @param startPoint Bezier curve starting point
 * @param controlPoint Bezier curve control point
 * @param endPoint Bezier curve ending point
 */
void GlyphCompositor::_detailBezier(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint) {
    std::map<float, glm::vec2> vertices{{ 1.f, glm::vec2(endPoint) }};
    this->_subdivide(startPoint, controlPoint, endPoint, 0.5f, 0.5f, vertices);

    for(std::map<float, glm::vec2>::iterator i = vertices.begin(); i != vertices.end(); i++) {
        uint32_t lastVertexIndex = this->_getVertexIndex(this->_currentGlyph.getVertices(), this->_currentGlyphData.lastVertex);

        glm::vec2 newVertex = i->second;
        int newVertexIndex = this->_getVertexIndex(this->_currentGlyph.getVertices(), newVertex);

        if(newVertexIndex == -1) {
            this->_currentGlyph.addVertex(newVertex);
            this->_currentGlyph.addEdge(vft::Edge{lastVertexIndex, this->_currentGlyphData.vertexId});
            this->_currentGlyphData.vertexId++;
        }
        else if(lastVertexIndex != newVertexIndex) {
            this->_currentGlyph.addEdge(vft::Edge{lastVertexIndex, static_cast<uint32_t>(newVertexIndex)});
        }

        this->_currentGlyphData.lastVertex = newVertex;
    }
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
void GlyphCompositor::_subdivide(glm::vec2 startPoint, glm::vec2 controlPoint, glm::vec2 endPoint, float t, float delta, std::map<float, glm::vec2> &vertices) {
    // Add current point
    glm::vec2 newVertex{(1 - t) * (1 - t) * startPoint + 2 * (1 - t) * t * controlPoint + (t * t) * endPoint};
    vertices.insert({t, newVertex});

    // Compute points around the current point with the given delta
    glm::vec2 left{(1 - (t - delta)) * (1 - (t - delta)) * startPoint + 2 * (1 - (t - delta)) * (t - delta) * controlPoint + (t - delta) * (t - delta) * endPoint};
    glm::vec2 right{(1 - (t + delta)) * (1 - (t + delta)) * startPoint + 2 * (1 - (t + delta)) * (t + delta) * controlPoint + (t + delta) * (t + delta) * endPoint};

    // Check if distance between curve(t - delta) and curve(t) is less than one pixel
    // or if points can be connected by a line without losing quality
    if(glm::length(newVertex - left) >= 64.f && newVertex.x != left.x && newVertex.y != left.y) {
        // The segment curve(t - delta) and curve(t) should be subdivided
        this->_subdivide(startPoint, controlPoint, endPoint, t - delta / 2.f, delta / 2.f, vertices);
    }

    // Check if distance between curve(t) and curve(t + delta) is less than one pixel
    // or if points can be connected by a line without losing quality
    if(glm::length(newVertex - right) >= 64.f && newVertex.x != right.x && newVertex.y != right.y) {
        // The segment curve(t) and curve(t + delta) should be subdivided
        this->_subdivide(startPoint, controlPoint, endPoint, t + delta / 2.f, delta / 2.f, vertices);
    }
}

/**
 * @brief Creates a triangulated glyph if not in cache and inserts glyph into cache
 * 
 * @param codePoint Unicode code point of glyph to triangulate
 * @param font Font to use for triangulation
 */
void GlyphCompositor::compose(uint32_t codePoint, std::shared_ptr<Font> font) {
    this->_currentGlyphData = vft::ComposedGlyphData{};

    // If code point is TAB, then it is rendered as four SPACE characters
    uint32_t newCodePoint = codePoint == vft::U_TAB ? vft::U_SPACE : codePoint;

    if(!font->getAllGlyphInfo().contains(newCodePoint)) {
        this->_composeGlyph(newCodePoint, font);
        this->_triangulate();

        // Insert glyph info to map
        font->setGlyphInfo(codePoint, this->_currentGlyph);
    }
}

/**
 * @brief Composes a glyph into a set of points and edges
 *
 * @param codePoint Unicode code point of glyph
 * @param font Font to use for triangulation
 */
void GlyphCompositor::_composeGlyph(uint32_t codePoint, std::shared_ptr<Font> font) {
    // Get glyph from .ttf file
    if(FT_Load_Char(font->getFace(), codePoint, FT_LOAD_NO_SCALE)) {
        throw std::runtime_error("Error loading glyph");
    }
    FT_GlyphSlot slot = font->getFace()->glyph;

    // Decompose outlines to vertices and vertex indices
    this->_currentGlyph = Glyph{};
    FT_Outline_Funcs outlineFunctions = {
        .move_to = this->_moveToFunc,
        .line_to = this->_lineToFunc,
        .conic_to = this->_conicToFunc,
        .cubic_to = this->_cubicToFunc,
        .shift = 0,
        .delta = 0
    };
    FT_Outline_Decompose(&(slot->outline), &outlineFunctions, this);
    if(this->_currentGlyph.getVertexCount() > 0) {
        // Make last contour closed
        this->_currentGlyph.updateEdge(
            this->_currentGlyph.getEdgeCount() - 1,
            vft::Edge{this->_currentGlyph.getEdges().back().first, this->_currentGlyphData.contourStartVertexId});

        // Combine final two contours into one
        if(this->_currentGlyphData.contourCount >= 2) {
            std::vector<glm::vec2> vertices = this->_currentGlyph.getVertices();
            std::vector<vft::Edge> edges = this->_currentGlyph.getEdges();

            std::set<uint32_t> intersections;
            this->_removeInverseEdges(edges, intersections);
            intersections = this->_resolveIntersectingEdges(vertices, edges);
            this->_removeInverseEdges(edges, intersections);
            this->_resolveSharedVertices(vertices, edges, intersections);
            this->_walkContours(vertices, edges, intersections);

            // Remove duplicate vertices
            CDT::DuplicatesInfo info = CDT::RemoveDuplicatesAndRemapEdges<float>(
                vertices,
                [](const glm::vec2 &p) { return p.x; },
                [](const glm::vec2 &p) { return p.y; },
                edges.begin(),
                edges.end(),
                [](const vft::Edge &e) { return e.first; },
                [](const vft::Edge &e) { return e.second; },
                [](uint32_t i1, uint32_t i2) -> vft::Edge { return vft::Edge{i1, i2}; });
            this->_currentGlyphData.vertexId -= info.duplicates.size();;

            this->_currentGlyph.setVertices(vertices);
            this->_currentGlyph.setEdges(edges);
        }
    }

    this->_currentGlyph.setAdvanceX(slot->advance.x);
    this->_currentGlyph.setAdvanceY(slot->advance.y);
}

/**
 * @brief Performs constrained delaunay triangulation on a glyph
 */
void GlyphCompositor::_triangulate() {
    // CDT uses Constrained Delaunay Triangulation algorithm
    CDT::Triangulation<float> cdt{
        CDT::VertexInsertionOrder::Auto,
        CDT::IntersectingConstraintEdges::TryResolve,
        1.0e-6};

    cdt.insertVertices(
        this->_currentGlyph.getVertices().begin(),
        this->_currentGlyph.getVertices().end(),
        [](const glm::vec2 &p) { return p.x; },
        [](const glm::vec2 &p) { return p.y; });
    cdt.insertEdges(
        this->_currentGlyph.getEdges().begin(),
        this->_currentGlyph.getEdges().end(),
        [](const vft::Edge &e) { return e.first; },
        [](const vft::Edge &e) { return e.second; });
    cdt.eraseOuterTrianglesAndHoles();

    // Create index buffer from triangulation
    CDT::TriangleVec cdtTriangles = cdt.triangles;
    for(int i = 0; i < cdtTriangles.size(); i++) {
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(0));
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(1));
        this->_currentGlyph.addIndex(cdtTriangles.at(i).vertices.at(2));
    }
}

/**
 * @brief Return the determinant of a 2x2 matrix
 * 
 * @param a 
 * @param b 
 * @param c 
 * @param d
 * 
 * @return Determinant
 */
double GlyphCompositor::_determinant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

/**
 * @brief Check if two edges intersect
 * 
 * @param vertices Vertex buffer
 * @param first First edge
 * @param second Second edge
 * @param intersection Computed point of intersection
 * 
 * @return True if edges intersect in edge boundaries, else false
 */
bool GlyphCompositor::_intersect(const std::vector<glm::vec2> &vertices, vft::Edge first, vft::Edge second, glm::vec2 &intersection) {
    static double epsilon = 1e-6;

    double x1 = vertices.at(first.first).x;
    double y1 = vertices.at(first.first).y;
    double x2 = vertices.at(first.second).x;
    double y2 = vertices.at(first.second).y;
    double x3 = vertices.at(second.first).x;
    double y3 = vertices.at(second.first).y;
    double x4 = vertices.at(second.second).x;
    double y4 = vertices.at(second.second).y;

    double det1 = this->_determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double det2 = this->_determinant(x1 - x3, y1 - y3, x3 - x4, y3 - y4);

    // Check if edges are parallel
    if(fabs(det1) < epsilon || fabs(det2) < epsilon) {
        return false;
    }

    intersection.x = this->_determinant(this->_determinant(x1, y1, x2, y2), x1 - x2, this->_determinant(x3, y3, x4, y4), x3 - x4) / det1;
    intersection.y = this->_determinant(this->_determinant(x1, y1, x2, y2), y1 - y2, this->_determinant(x3, y3, x4, y4), y3 - y4) / det1;

    // Check if intersection is in the boundaries of both edges
    if(
        (
            (intersection.x >= std::min(x1, x2) && intersection.x <= std::max(x1, x2)) &&
            (intersection.y >= std::min(y1, y2) && intersection.y <= std::max(y1, y2))
        ) &&
        (
            (intersection.x >= std::min(x3, x4) && intersection.x <= std::max(x3, x4)) &&
            (intersection.y >= std::min(y3, y4) && intersection.y <= std::max(y3, y4))
        )
    ) {
        return true;
    }

    return false;
}

/**
 * @brief Resolve intersecting edges by creating vertex at intersection and updating edges
 * 
 * @param vertices Vertices of glyph
 * @param edges Edges of glyph
 * 
 * @return Set of vertex indices that are intersections
 */
std::set<uint32_t> GlyphCompositor::_resolveIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges) {
    std::set<uint32_t> intersectionVertexIndices;

    for(int i = 0; i < edges.size(); i++) {
        for(int j = 0; j < edges.size(); j++) {
            if(i == j) {
                continue;
            }

            vft::Edge &firstEdge = edges.at(i);
            vft::Edge &secondEdge = edges.at(j);

            if(firstEdge == secondEdge) {
                // Add both intersection vertices
                intersectionVertexIndices.insert(firstEdge.first);
                intersectionVertexIndices.insert(firstEdge.second);
                continue;
            }
            else if(
                firstEdge.isInverse(secondEdge) ||
                firstEdge.first == secondEdge.first ||
                firstEdge.first == secondEdge.second ||
                firstEdge.second == secondEdge.first ||
                firstEdge.second == secondEdge.second
            ) {
                continue;
            }

            glm::vec2 intersection = glm::vec2(0.f, 0.f);
            bool intersect = this->_intersect(vertices, firstEdge, secondEdge, intersection);
            if(intersect) {
                if(glm::distance(intersection, vertices.at(firstEdge.first)) <= 1.f) {
                    uint32_t firstEdgeFirstIndex = firstEdge.first;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, vft::Edge{firstEdge.first, secondEdge.second});
                    edges.at(j).second = firstEdgeFirstIndex;

                    intersectionVertexIndices.insert(firstEdgeFirstIndex);
                }
                else if(glm::distance(intersection, vertices.at(firstEdge.second)) <= 1.f) {
                    uint32_t firstEdgeSecondIndex = firstEdge.second;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, vft::Edge{firstEdge.second, secondEdge.second});
                    edges.at(j).second = firstEdgeSecondIndex;

                    intersectionVertexIndices.insert(firstEdgeSecondIndex);
                }
                else if(glm::distance(intersection, vertices.at(secondEdge.first)) <= 1.f) {
                    uint32_t secondEdgeFirstIndex = secondEdge.first;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, vft::Edge{secondEdge.first, firstEdge.second});
                    edges.at(i).second = secondEdgeFirstIndex;

                    intersectionVertexIndices.insert(secondEdgeFirstIndex);
                }
                else if(glm::distance(intersection, vertices.at(secondEdge.second)) <= 1.f) {
                    uint32_t secondEdgeSecondIndex = secondEdge.second;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, vft::Edge{secondEdge.second, firstEdge.second});
                    edges.at(i).second = secondEdgeSecondIndex;

                    intersectionVertexIndices.insert(secondEdgeSecondIndex);
                }
                else {
                    uint32_t intersectionIndex = vertices.size();
                    vertices.push_back(intersection);
                    this->_currentGlyphData.vertexId++;

                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    firstEdge.second = intersectionIndex;
                    secondEdge.second = intersectionIndex;

                    vft::Edge fe = edges.at(i);
                    vft::Edge se = edges.at(j);

                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), fe);
                    edges.insert(firstEdgeIterator + 1, vft::Edge{intersectionIndex, oldFirstEdgeSecondIndex});

                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), se);
                    edges.insert(secondEdgeIterator + 1, vft::Edge{intersectionIndex, oldSecondEdgeSecondIndex});

                    intersectionVertexIndices.insert(intersectionIndex);
                }
            }
        }
    }

    return intersectionVertexIndices;
}

/**
 * @brief Resolve intersections that only include one vertex (polygons intersect only in vertex)
 * 
 * @param vertices Vertices of glyph
 * @param edges Edges of glyph
 * @param intersections Intersection indices
 */
void GlyphCompositor::_resolveSharedVertices(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges, std::set<uint32_t> &intersections) {
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            vft::Edge &firstEdge = edges.at(i);
            vft::Edge &secondEdge = edges.at(j);

            if(
                firstEdge == secondEdge ||
                firstEdge.isInverse(secondEdge) ||
                firstEdge.first == secondEdge.first ||
                firstEdge.first == secondEdge.second ||
                firstEdge.second == secondEdge.first
            ) {
                continue;
            }
            else if(firstEdge.second == secondEdge.second) {
                // Vertex index at which two edges intersect at end
                uint32_t intersectionIndex = firstEdge.second;

                // Check if it is only one point of intersection
                std::vector<uint32_t> edgeIndices = this->_getEdgesStartingAt(intersectionIndex, edges);
                if(edgeIndices.size() == 1) {
                    continue;
                }
                else if(edgeIndices.size() < 1) {
                    throw std::runtime_error("_resolveSharedVertices(): Invalid edges");
                }

                uint32_t firstVertexAfterIntersection = edges.at(edgeIndices.at(0)).second;
                uint32_t secondVertexAfterIntersection = edges.at(edgeIndices.at(1)).second;

                if(intersections.find(intersectionIndex) != intersections.end()) {
                    // This vertex is an intersection, not a shared vertex of polygons
                    continue;
                }

                // Create second vertex which is identical to intersection
                uint32_t duplicateIntersectionIndex = this->_currentGlyphData.vertexId++;
                vertices.push_back(vertices.at(intersectionIndex));

                // Update starting vertex of edge that comes after intersected edge in first polygon
                edgeIndices = this->_getEdgesStartingAt(intersectionIndex, edges);
                edges.at(edgeIndices.at(0)).first = duplicateIntersectionIndex;

                // Update ending vertex of intersected edge in second polygon
                secondEdge.second = duplicateIntersectionIndex;
            }
        }
    }
}

/**
 * @brief Removes one of the two same edges
 * 
 * @param edges Vector of edges
 */
void GlyphCompositor::_removeDuplicateEdges(std::vector<vft::Edge> &edges) {
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            if(edges.at(i) == edges.at(j)) {
                edges.erase(std::find(edges.begin(), edges.end(), edges.at(j)));

                j--;
                break;
            }
        }
    }
}

/**
 * @brief Removes both edges where first edge starts at the end of the second edge and first edge ends at the start of second edge and remove intesections
 * 
 * @param edges Vector of edges
 * @param intersections Set of intesections
 */
void GlyphCompositor::_removeInverseEdges(std::vector<vft::Edge> &edges, std::set<uint32_t> &intersections) {
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            if(edges.at(i).isInverse(edges.at(j))) {
                intersections.erase(edges.at(i).first);
                intersections.erase(edges.at(i).second);

                edges.erase(std::find(edges.begin(), edges.end(), edges.at(i)));
                edges.erase(std::find(edges.begin(), edges.end(), edges.at(j - 1)));

                i--;
                break;
            }
        }
    }
}

/**
 * @brief Indicates whether vertex is on the left side of edge
 * 
 * @param startVertex Starting vertex of edge
 * @param endVertex Ending vertex of edge
 * @param vertex Vertex to examine
 * 
 * @return True if vertex is on the left side of edge, else false
 */
bool GlyphCompositor::_isInPositiveRegion(glm::vec2 startVertex, glm::vec2 endVertex, glm::vec2 vertex) {
    float a = endVertex.y - startVertex.y;
    float b = startVertex.x - endVertex.x;
    float c = endVertex.x * startVertex.y - startVertex.x * endVertex.y;
    float d = a * vertex.x + b * vertex.y + c;

    return d < 0;
}

/**
 * @brief Creates non-intersecting polygons based on edges
 * 
 * @param vertices Vertices of glyph
 * @param edges Edges of glyph
 * @param intersections Intersection indices
 */
void GlyphCompositor::_walkContours(const std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges, std::set<uint32_t> intersections) {
    if(edges.size() == 0) {
        return;
    }

    std::vector<vft::Edge> newEdges;
    std::set<uint32_t> visitedEdges;
    std::set<uint32_t> visitedIntersections;

    while(visitedIntersections.size() != intersections.size()) {
        // Set starting vertex as intersection vertex which was not yet visited
        uint32_t currentVertexIndex = 0;
        for(uint32_t vertexIndex : intersections) {
            if(visitedIntersections.find(vertexIndex) == visitedIntersections.end()) {
                // vertexIndex was not yet visited
                currentVertexIndex = vertexIndex;
                break;
            }
        }
        visitedIntersections.insert(currentVertexIndex);

        // Get all edges starting at currentVertex
        std::vector<uint32_t> edgeIndices = this->_getEdgesStartingAt(currentVertexIndex, edges);

        if(this->_isInPositiveRegion(vertices.at(edges.at(edgeIndices.at(0)).first), vertices.at(edges.at(edgeIndices.at(0)).second), vertices.at(edges.at(edgeIndices.at(1)).second))) {
            newEdges.push_back(edges.at(edgeIndices.at(1)));
            visitedEdges.insert(edgeIndices.at(1));
            currentVertexIndex = edges.at(edgeIndices.at(1)).second;
            this->_visitEdgesUntilIntersection(edges, visitedEdges, edgeIndices.at(0));
        }
        else {
            newEdges.push_back(edges.at(edgeIndices.at(0)));
            visitedEdges.insert(edgeIndices.at(0));
            currentVertexIndex = edges.at(edgeIndices.at(0)).second;
            this->_visitEdgesUntilIntersection(edges, visitedEdges, edgeIndices.at(1));
        }

        while(true) {
            edgeIndices = this->_getEdgesStartingAt(currentVertexIndex, edges);
            if(edgeIndices.size() == 1) {
                if(visitedEdges.contains(edgeIndices.at(0))) {
                    break;
                }

                newEdges.push_back(edges.at(edgeIndices.at(0)));
                visitedEdges.insert(edgeIndices.at(0));
                currentVertexIndex = edges.at(edgeIndices.at(0)).second;
            }
            else if(edgeIndices.size() == 2) {
                visitedIntersections.insert(currentVertexIndex);

                if(this->_isInPositiveRegion(vertices.at(edges.at(edgeIndices.at(0)).first), vertices.at(edges.at(edgeIndices.at(0)).second), vertices.at(edges.at(edgeIndices.at(1)).second))) {
                    if(visitedEdges.contains(edgeIndices.at(1))) {
                        break;
                    }

                    newEdges.push_back(edges.at(edgeIndices.at(1)));
                    visitedEdges.insert(edgeIndices.at(1));
                    currentVertexIndex = edges.at(edgeIndices.at(1)).second;
                    this->_visitEdgesUntilIntersection(edges, visitedEdges, edgeIndices.at(0));
                }
                else {
                    if(visitedEdges.contains(edgeIndices.at(0))) {
                        break;
                    }

                    newEdges.push_back(edges.at(edgeIndices.at(0)));
                    visitedEdges.insert(edgeIndices.at(0));
                    currentVertexIndex = edges.at(edgeIndices.at(0)).second;
                    this->_visitEdgesUntilIntersection(edges, visitedEdges, edgeIndices.at(1));
                }
            }
            else {
                throw std::runtime_error("_walkContours(): Invalid edges.");
            }
        }
    }

    if(visitedEdges.size() < edges.size()) {
        // Find first unvisited edge and start from first vertex
        uint32_t edgeIndex = 0;
        while(visitedEdges.find(edgeIndex) != visitedEdges.end()) {
            edgeIndex++;
        }
        uint32_t currentVertexIndex = edges.at(edgeIndex).first;

        // Add unvisited edges to output
        while(visitedEdges.size() < edges.size()) {
            std::vector<uint32_t> edgeIndices = this->_getEdgesStartingAt(currentVertexIndex, edges);

            // Stay on same contour
            if(edgeIndices.size() == 1) {
                if(visitedEdges.contains(edgeIndices.at(0))) {
                    uint32_t edgeIndex = 0;
                    while(visitedEdges.find(edgeIndex) != visitedEdges.end()) {
                        edgeIndex++;
                    }
                    currentVertexIndex = edges.at(edgeIndex).first;

                    continue;
                }

                newEdges.push_back(edges.at(edgeIndices.at(0)));
                visitedEdges.insert(edgeIndices.at(0));
                currentVertexIndex = edges.at(edgeIndices.at(0)).second;
            }
            // Go to another contour
            else if(edgeIndices.size() == 0) {
                uint32_t edgeIndex = 0;
                while(visitedEdges.find(edgeIndex) != visitedEdges.end()) {
                    edgeIndex++;
                }
                currentVertexIndex = edges.at(edgeIndex).first;
            }
            else {
                throw std::runtime_error("_walkContours(): Invalid edges.");
            }
        }
    }

    edges = newEdges;
}

/**
 * @brief Visit edges that are not in final polygon
 *
 * @param edges Vector of edges
 * @param visited Set of visited edges
 * @param startEdgeIndex Index of edge where to start
 */
void GlyphCompositor::_visitEdgesUntilIntersection(std::vector<vft::Edge> &edges, std::set<uint32_t> &visited, uint32_t startEdgeIndex) {
    visited.insert(startEdgeIndex);
    uint32_t currentVertexId = edges.at(startEdgeIndex).second;
    uint32_t lastEdgeIndex = startEdgeIndex;

    std::vector<uint32_t> edgeIndices = this->_getEdgesStartingAt(currentVertexId, edges);
    while(true) {
        if(edgeIndices.size() == 1) {
            // Stay on same contour
            visited.insert(edgeIndices.at(0));
            currentVertexId = edges.at(edgeIndices.at(0)).second;
            lastEdgeIndex = edgeIndices.at(0);
        }
        else {
            // Intersection
            return;
        }

        edgeIndices = this->_getEdgesStartingAt(currentVertexId, edges);
    }
}

/**
 * @brief Get all edges starting at given vertex
 * 
 * @param vertexId Index of starting vertex
 * @param edges Vector of all edges
 * 
 * @return Vector of all edges starting at given vertex
 */
std::vector<uint32_t> GlyphCompositor::_getEdgesStartingAt(uint32_t vertexId, const std::vector<vft::Edge> &edges) {
    std::vector<uint32_t> indices;
    for(int i = 0; i < edges.size(); i++) {
        if(edges.at(i).first == vertexId) {
            indices.push_back(i);
        }
    }

    return indices;
}

}
