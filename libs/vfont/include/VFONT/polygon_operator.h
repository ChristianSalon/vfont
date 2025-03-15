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

#include <glm/vec2.hpp>

#include "circular_dll.h"
#include "text_renderer_utils.h"

namespace vft {

class Contour {
public:
    bool visited;
    CircularDLL<Edge> list;

    Contour(bool visited, CircularDLL<Edge> list) : visited{visited}, list{list} {}
};

class PolygonOperator {
public:
    PolygonOperator() = default;
    ~PolygonOperator() = default;

protected:
    double _epsilon{1e-6};

    std::vector<glm::vec2> _vertices{};
    std::vector<Contour> _first{};
    std::vector<Contour> _second{};
    std::vector<CircularDLL<Edge>> _output{};

    std::list<uint32_t> _intersections{};

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
    void _markContourAsVisited(CircularDLL<Edge>::Node *vertex);

    void _addIntersectionIfNeeded(std::list<uint32_t> &intersections, uint32_t intersection);
    void _removeUnwantedIntersections(std::list<uint32_t> &intersections, const std::vector<Contour> &contours);

    std::vector<CircularDLL<Edge>::Node *> _getEdgesStartingAt(uint32_t vertex);
    bool _isOnLeftSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point);
    double _determinant(double a, double b, double c, double d);
    bool _isEdgeOnEdge(Edge first, Edge second);
    bool _isPointOnEdge(glm::vec2 point, Edge edge);
};

}  // namespace vft
