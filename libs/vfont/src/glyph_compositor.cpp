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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

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

    if(!pThis->_currentGlyphData.isFirstContour) {
        // Makes the contour closed
        pThis->_currentGlyph.updateEdge(
            pThis->_currentGlyph.getEdgeCount() - 1,
            vft::Edge{pThis->_currentGlyph.getEdges().at(pThis->_currentGlyph.getEdgeCount() - 1).first, pThis->_currentGlyphData.contourStartVertexId});
    }
    else {
        pThis->_currentGlyphData.isFirstContour = false;
    }

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
        if(glm::distance(vertex, vertices.at(i)) <= 64.f) {
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

void GlyphCompositor::compose(uint32_t codePoint, std::shared_ptr<Font> font) {
    this->_currentGlyphData = vft::ComposedGlyphData{};

    // If code point is TAB, then it is rendered as four SPACE characters
    uint32_t newCodePoint = codePoint == vft::U_TAB ? vft::U_SPACE : codePoint;

    if(!font->getAllGlyphInfo().contains(newCodePoint)) {
        this->_composeGlyph(newCodePoint, font);
        this->_triangulate();

        // Insert glyph info to map
        font->setGlyphInfo(newCodePoint, this->_currentGlyph);
    }
}

/**
 * @brief Composes a glyph into a set of points and edges
 *
 * @param codePoint Unicode code point of glyph
 */
void GlyphCompositor::_composeGlyph(uint32_t codePoint, std::shared_ptr<Font> font) {
    // Get glyph from .ttf file
    if(FT_Load_Char(font->getFace(), codePoint, FT_LOAD_DEFAULT)) {
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
        this->_currentGlyph.updateEdge(
            this->_currentGlyph.getEdgeCount() - 1,
            vft::Edge{this->_currentGlyph.getEdges().back().first, this->_currentGlyphData.contourStartVertexId});
    }

    this->_currentGlyph.setAdvanceX(slot->advance.x >> 6);
    this->_currentGlyph.setAdvanceY(slot->advance.y >> 6);
}

/**
 * @brief Performs constrained delaunay triangulation on a decomposed glyph
 */
void GlyphCompositor::_triangulate() {
    std::vector<glm::vec2> vertices = this->_currentGlyph.getVertices();
    std::vector<vft::Edge> edges = this->_currentGlyph.getEdges();

    // Remove duplicate vertices
    CDT::RemoveDuplicatesAndRemapEdges<float>(
        vertices,
        [](const glm::vec2 &p) { return p.x; },
        [](const glm::vec2 &p) { return p.y; },
        edges.begin(),
        edges.end(),
        [](const vft::Edge &e) { return e.first; },
        [](const vft::Edge &e) { return e.second; },
        [](uint32_t i1, uint32_t i2) -> vft::Edge { return vft::Edge{i1, i2}; });
    this->_removeDuplicateEdges(edges);
    this->_resolveIntersectingEdges(vertices, edges);
    // Combine intersecting contours into one
    this->_walkContours(vertices, edges);

    this->_currentGlyph.setVertices(vertices);
    this->_currentGlyph.setEdges(edges);

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

double GlyphCompositor::_determinant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

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

    return this->_isPointOnLineSegment(x1, y1, x2, y2, intersection.x, intersection.y) &&
        this->_isPointOnLineSegment(x3, y3, x4, y4, intersection.x, intersection.y);
}

bool GlyphCompositor::_isPointOnLineSegment(double x1, double y1, double x2, double y2, double x, double y) {
    static double epsilon = 1e-6;

    return fabs(this->_determinant(x - x1, y - y1, x2 - x1, y2 - y1)) < epsilon && (x - x1) * (x - x2) + (y - y1) * (y - y2) <= 0;
}

void GlyphCompositor::_checkIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges, vft::Edge &edge) {
    for(int i = 0; i < edges.size(); i++) {
        vft::Edge &firstEdge = edge;
        vft::Edge &secondEdge = edges.at(i);

        if(
            firstEdge == secondEdge ||
            firstEdge.second == secondEdge.first ||
            firstEdge.first == secondEdge.second ||
            firstEdge.first == secondEdge.first ||
            firstEdge.second == secondEdge.second) {
            continue;
        }

        glm::vec2 intersection = glm::vec2(0.f, 0.f);
        bool intersect = this->_intersect(vertices, firstEdge, secondEdge, intersection);
        if(intersect) {
            if(intersection == vertices.at(firstEdge.first)) {
                uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                secondEdge.second = firstEdge.first;
                auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                edges.insert(secondEdgeIterator + 1, vft::Edge{firstEdge.first, oldSecondEdgeSecondIndex});
            }
            else if(intersection == vertices.at(firstEdge.second)) {
                uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                secondEdge.second = firstEdge.second;
                auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                edges.insert(secondEdgeIterator + 1, vft::Edge{firstEdge.second, oldSecondEdgeSecondIndex});
            }
            else if(intersection == vertices.at(secondEdge.first)) {
                uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                firstEdge.second = secondEdge.first;
                auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                edges.insert(firstEdgeIterator + 1, vft::Edge{secondEdge.first, oldFirstEdgeSecondIndex});
            }
            else if(intersection == vertices.at(secondEdge.second)) {
                uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                firstEdge.second = secondEdge.second;
                auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                edges.insert(firstEdgeIterator + 1, vft::Edge{secondEdge.second, oldFirstEdgeSecondIndex});
            }
            else {
                uint32_t intersectionIndex = vertices.size();
                vertices.push_back(intersection);

                uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                firstEdge.second = intersectionIndex;
                secondEdge.second = intersectionIndex;

                auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                edges.insert(firstEdgeIterator + 1, vft::Edge{intersectionIndex, oldFirstEdgeSecondIndex});

                secondEdge = edges.at(i + 1);
                auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                edges.insert(secondEdgeIterator + 1, vft::Edge{intersectionIndex, oldSecondEdgeSecondIndex});

                i++;
            }

            i++;
        }
    }
}

void GlyphCompositor::_resolveIntersectingEdges(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges) {
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            vft::Edge &firstEdge = edges.at(i);
            vft::Edge &secondEdge = edges.at(j);

            if(
                firstEdge == secondEdge ||
                firstEdge.isInverse(secondEdge) ||
                firstEdge.first == secondEdge.first ||
                firstEdge.first == secondEdge.second ||
                firstEdge.second == secondEdge.first ||
                firstEdge.second == secondEdge.second) {
                continue;
            }

            glm::vec2 intersection = glm::vec2(0.f, 0.f);
            bool intersect = this->_intersect(vertices, firstEdge, secondEdge, intersection);
            if(intersect) {
                if(intersection == vertices.at(firstEdge.first)) {
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    secondEdge.second = firstEdge.first;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {firstEdge.first, oldSecondEdgeSecondIndex});
                }
                else if(intersection == vertices.at(firstEdge.second)) {
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    secondEdge.second = firstEdge.second;
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {firstEdge.second, oldSecondEdgeSecondIndex});
                }
                else if(intersection == vertices.at(secondEdge.first)) {
                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    firstEdge.second = secondEdge.first;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {secondEdge.first, oldFirstEdgeSecondIndex});
                }
                else if(intersection == vertices.at(secondEdge.second)) {
                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    firstEdge.second = secondEdge.second;
                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {secondEdge.second, oldFirstEdgeSecondIndex});
                }
                else {
                    uint32_t intersectionIndex = vertices.size();
                    vertices.push_back(intersection);

                    uint32_t oldFirstEdgeSecondIndex = firstEdge.second;
                    uint32_t oldSecondEdgeSecondIndex = secondEdge.second;
                    firstEdge.second = intersectionIndex;
                    secondEdge.second = intersectionIndex;

                    auto firstEdgeIterator = std::find(edges.begin(), edges.end(), firstEdge);
                    edges.insert(firstEdgeIterator + 1, {intersectionIndex, oldFirstEdgeSecondIndex});

                    secondEdge = edges.at(j + 1);
                    auto secondEdgeIterator = std::find(edges.begin(), edges.end(), secondEdge);
                    edges.insert(secondEdgeIterator + 1, {intersectionIndex, oldSecondEdgeSecondIndex});

                    j++;
                }

                j++;
            }
        }
    }
}

void GlyphCompositor::_removeDuplicateEdges(std::vector<vft::Edge> &edges) {
    for(int i = 0; i < edges.size(); i++) {
        for(int j = i + 1; j < edges.size(); j++) {
            if(edges.at(i).isInverse(edges.at(j))) {
                edges.erase(std::find(edges.begin(), edges.end(), edges.at(i)));
                edges.erase(std::find(edges.begin(), edges.end(), edges.at(j - 1)));

                i--;
                break;
            }
        }
    }
}

void GlyphCompositor::_walkContours(const std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges) {
    if(edges.size() == 0) {
        return;
    }

    std::vector<vft::Edge> newEdges = {edges.at(0)};
    std::set<uint32_t> visitedEdgesIndices = {0};

    while(visitedEdgesIndices.size() < edges.size()) {
        uint32_t currentVertexId = newEdges.at(newEdges.size() - 1).second;
        std::vector<uint32_t> edgeIndices = this->_findAllEdgesContainingVertex(currentVertexId, edges);
        if(edgeIndices.size() == 2) {
            // Switch contour
            if(edges.at(edgeIndices.at(0) - 1) == newEdges.at(newEdges.size() - 1)) {
                // Edge at index edgeIndices[0] is next edge of contour without switch
                // Edge at index edgeIndices[1] is next edge of contour after switch
                newEdges.push_back(edges.at(edgeIndices.at(1)));
                visitedEdgesIndices.insert(edgeIndices.at(1));
                this->_updateVisitedEdgesUntilNextIntersection(edges, visitedEdgesIndices, edgeIndices.at(0));
            }
            else {
                // Edge at index edgeIndices[0] is next edge of contour after switch
                // Edge at index edgeIndices[1] is next edge of contour without switch
                newEdges.push_back(edges.at(edgeIndices.at(0)));
                visitedEdgesIndices.insert(edgeIndices.at(0));
                this->_updateVisitedEdgesUntilNextIntersection(edges, visitedEdgesIndices, edgeIndices.at(1));
            }
        }
        else if(edgeIndices.size() == 1) {
            // Stay on same contour
            if(visitedEdgesIndices.contains(edgeIndices.at(0))) {
                uint32_t newEdgeIndex = *(visitedEdgesIndices.rbegin()) + 1;
                newEdges.push_back(edges.at(newEdgeIndex));
                visitedEdgesIndices.insert(newEdgeIndex);
                continue;
            }

            newEdges.push_back(edges.at(edgeIndices.at(0)));
            visitedEdgesIndices.insert(edgeIndices.at(0));
        }
        else {
            /*float maxAngle = 0.f;
            int maxIndex = 0;
            glm::vec2 lastEdgeVector = glm::normalize(glm::vec2(vertices.at(newEdges.at(newEdges.size() - 1).first), vertices.at(newEdges.at(newEdges.size() - 1).second)));
            for(int i = 0; i < edgeIndices.size(); i++) {
                uint32_t edgeIndex = edgeIndices.at(i);
                if(!(edges.at(edgeIndex - 1) == newEdges.at(newEdges.size() - 1))) {
                    newEdges.push_back(edges.at(edgeIndex));
                    visitedEdgesIndices.insert(edgeIndex);

                    glm::vec2 nextEdgeVector = glm::normalize(glm::vec2(vertices.at(edges.at(edgeIndex).first), vertices.at(edges.at(edgeIndex).second)));
                    float angle = glm::angle(lastEdgeVector, nextEdgeVector);
                    if(angle > maxAngle) {
                        maxAngle = angle;
                        maxIndex = i;
                    }
                }
            }

            for(int i = 0; i < edgeIndices.size(); i++) {
                if(i == maxIndex) {
                    newEdges.push_back(edges.at(maxIndex));
                    visitedEdgesIndices.insert(maxIndex);
                }
                else {
                    this->_updateVisitedEdgesUntilNextIntersection(edges, visitedEdgesIndices, i);
                }
            }
            */
        }

        // Visited last edge and is closed
        /*uint32_t maxVisitedIndex = *(visitedEdgesIndices.rbegin());
        if(maxVisitedIndex == edges.size() - 1) {
            break;
        }*/
    }

    edges = newEdges;
}

void GlyphCompositor::_updateVisitedEdgesUntilNextIntersection(std::vector<vft::Edge> &edges, std::set<uint32_t> &visited, uint32_t startEdgeIndex) {
    visited.insert(startEdgeIndex);
    uint32_t currentVertexId = edges.at(startEdgeIndex).second;

    std::vector<uint32_t> edgeIndices = this->_findAllEdgesContainingVertex(currentVertexId, edges);
    while(true) {
        if(edgeIndices.size() == 1) {
            // Stay on same contour
            visited.insert(edgeIndices.at(0));
            currentVertexId = edges.at(edgeIndices.at(0)).second;
        }
        else if(edgeIndices.size() >= 2) {
            return;
        }
        else {
            throw std::runtime_error("Invalid edges.");
        }

        edgeIndices = this->_findAllEdgesContainingVertex(currentVertexId, edges);
    }
}

std::vector<uint32_t> GlyphCompositor::_findAllEdgesContainingVertex(uint32_t vertexId, const std::vector<vft::Edge> &edges) {
    std::vector<uint32_t> indices;
    for(int i = 0; i < edges.size(); i++) {
        if(edges.at(i).first == vertexId) {
            indices.push_back(i);
        }
    }

    return indices;
}

}
