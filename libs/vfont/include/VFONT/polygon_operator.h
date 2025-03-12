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
    CircularDLL<uint32_t> list;

    Contour(bool visited, CircularDLL<uint32_t> list) : visited{visited}, list{list} {}
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
    std::vector<CircularDLL<uint32_t>> _output{};

    std::list<uint32_t> _normalIntersections{};
    std::list<uint32_t> _specialIntersections{};

public:
    void join(const std::vector<glm::vec2> &vertices,
              const std::vector<CircularDLL<uint32_t>> &first,
              const std::vector<CircularDLL<uint32_t>> &second);

    void setEpsilon(double epsilon);

    std::vector<glm::vec2> getVertices();
    std::vector<CircularDLL<uint32_t>> getPolygon();

protected:
    void _initializeContours(const std::vector<glm::vec2> &vertices,
                             const std::vector<CircularDLL<uint32_t>> &first,
                             const std::vector<CircularDLL<uint32_t>> &second);

    void _resolveIntersectingEdges();
    bool _intersect(Edge first, Edge second, glm::vec2 &intersection);

    void _walkContours();
    uint32_t _walkUntilIntersectionOrStart(CircularDLL<uint32_t>::Node *start, unsigned int contourIndex);
    void _markContourAsVisited(CircularDLL<uint32_t>::Node *vertex);

    std::vector<CircularDLL<uint32_t>::Node *> _getEdgesStartingAt(uint32_t vertex);
    bool _isOnLeftSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point);
    double _determinant(double a, double b, double c, double d);
    bool _isEdgeOnEdge(Edge first, Edge second);
    bool _isPointOnEdge(glm::vec2 point, Edge edge);
};

}  // namespace vft
