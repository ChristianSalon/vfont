/**
 * @file polygon_operator.h
 * @author Christian Saloň
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <list>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>

#include "circular_dll.h"
#include "edge.h"

namespace vft {

/**
 * @brief Represents a contour of a polygon (glyph)
 */
class Contour {
public:
    bool visited;           /**< True if contour was processed, else false */
    CircularDLL<Edge> list; /**< Edges of contour */

    Contour(bool visited, CircularDLL<Edge> list) : visited{visited}, list{list} {}
};

/**
 * @brief Performs boolean operations on two polygons which can have holes and self intersections
 */
class PolygonOperator {
public:
    PolygonOperator() = default;
    ~PolygonOperator() = default;

protected:
    double _epsilon{1e-6}; /**< Max error */

    std::vector<glm::vec2> _vertices{};       /**< Vertices of both polygons */
    std::vector<Contour> _first{};            /**< First polygon */
    std::vector<Contour> _second{};           /**< Second polygon */
    std::vector<CircularDLL<Edge>> _output{}; /**< Output polygon */

    std::list<uint32_t> _intersections{}; /**< Linked list of intersections between first and second polyogon */

public:
    void join(const std::vector<glm::vec2> &vertices,
              const std::vector<CircularDLL<Edge>> &first,
              const std::vector<CircularDLL<Edge>> &second);

    void setEpsilon(double epsilon);

    std::vector<glm::vec2> getVertices();
    std::vector<CircularDLL<Edge>> getPolygon();

protected:
    void _initializeContours(const std::vector<glm::vec2> &vertices,
                             const std::vector<CircularDLL<Edge>> &first,
                             const std::vector<CircularDLL<Edge>> &second);

    std::vector<CircularDLL<Edge>> _resolveSelfIntersections(CircularDLL<Edge> contour);
    void _resolveOverlappingEdges();
    void _resolveIntersectingEdges();
    bool _intersect(Edge first, Edge second, glm::vec2 &intersection);

    void _walkContours();
    uint32_t _walkUntilIntersectionOrStart(CircularDLL<Edge>::Node *start, unsigned int contourIndex);
    void _markContourAsVisited(CircularDLL<Edge>::Node *edge);

    void _addIntersectionIfNeeded(std::list<uint32_t> &intersections, uint32_t intersection);
    void _removeUnwantedIntersections(std::list<uint32_t> &intersections, const std::vector<Contour> &contours);

    std::vector<CircularDLL<Edge>::Node *> _getEdgesStartingAt(uint32_t vertex);
    bool _isOnLeftSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point);
    double _determinant(double a, double b, double c, double d);
    bool _isEdgeOnEdge(Edge first, Edge second);
    bool _isPointOnEdge(glm::vec2 point, Edge edge);
};

}  // namespace vft
