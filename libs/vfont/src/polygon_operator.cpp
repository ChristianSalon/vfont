/**
 * @file polygon_operator.cpp
 * @author Christian Saloň
 */

#include "polygon_operator.h"

namespace vft {

void PolygonOperator::join(const std::vector<glm::vec2> &vertices,
                           const std::vector<CircularDLL<uint32_t>> &first,
                           const std::vector<CircularDLL<uint32_t>> &second) {
    this->_initializeContours(vertices, first, second);
    this->_resolveIntersectingEdges();
    this->_walkContours();
}

void PolygonOperator::_initializeContours(const std::vector<glm::vec2> &vertices,
                                          const std::vector<CircularDLL<uint32_t>> &first,
                                          const std::vector<CircularDLL<uint32_t>> &second) {
    // Reset polygons
    this->_first.clear();
    this->_second.clear();
    this->_output.clear();

    // Initialize both polygons
    this->_vertices = vertices;
    for (CircularDLL<uint32_t> contour : first) {
        this->_first.push_back(Contour{false, contour});
    }
    for (CircularDLL<uint32_t> contour : second) {
        this->_second.push_back(Contour{false, contour});
    }
}

void PolygonOperator::_resolveIntersectingEdges() {
    for (Contour &firstPolygonContour : this->_first) {
        for (unsigned int i = 0; i < firstPolygonContour.list.size(); i++) {
            Edge firstPolygonEdge{firstPolygonContour.list.getAt(i)->value,
                                  firstPolygonContour.list.getAt(i)->next->value};

            for (Contour &secondPolygonContour : this->_second) {
                for (unsigned int j = 0; j < secondPolygonContour.list.size(); j++) {
                    Edge secondPolygonEdge{secondPolygonContour.list.getAt(j)->value,
                                           secondPolygonContour.list.getAt(j)->next->value};

                    // Skip inverse or subsequent edges
                    if (firstPolygonEdge.isInverse(secondPolygonEdge) ||
                        firstPolygonEdge.first == secondPolygonEdge.first ||
                        firstPolygonEdge.first == secondPolygonEdge.second ||
                        firstPolygonEdge.second == secondPolygonEdge.first ||
                        firstPolygonEdge.second == secondPolygonEdge.second) {
                        continue;
                    }

                    // Check if edges intersect
                    glm::vec2 intersection{0, 0};
                    if (this->_intersect(firstPolygonEdge, secondPolygonEdge, intersection)) {
                        if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                          this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon &&
                            glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                          this->_vertices.at(secondPolygonEdge.first)) <= this->_epsilon) {
                            // Fully overlapped edges (inverse edges)

                            // Update edges in second polygon
                            secondPolygonContour.list.getAt(j)->value = firstPolygonEdge.second;
                            secondPolygonContour.list.getAt(j)->next->value = firstPolygonEdge.first;

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.first);
                            this->_specialIntersections.push_back(firstPolygonEdge.second);
                        } else if (this->_isEdgeOnEdge(firstPolygonEdge, secondPolygonEdge)) {
                            // First edge fully lies on second edge

                            // Update edges in second polygon
                            secondPolygonContour.list.insertAt(firstPolygonEdge.second, j + 1);
                            secondPolygonContour.list.insertAt(firstPolygonEdge.first, j + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.second);
                            this->_specialIntersections.push_back(firstPolygonEdge.first);
                        } else if (this->_isEdgeOnEdge(secondPolygonEdge, firstPolygonEdge)) {
                            // Second edge fully lies on first edge

                            // Update edges in first polygon
                            firstPolygonContour.list.insertAt(secondPolygonEdge.second, i + 1);
                            firstPolygonContour.list.insertAt(secondPolygonEdge.first, i + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(secondPolygonEdge.second);
                            this->_specialIntersections.push_back(secondPolygonEdge.first);
                        } else if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                                 this->_vertices.at(secondPolygonEdge.first)) <= this->_epsilon) {
                            // Polygons have one vertex at same positions

                            // Update edges in both polygons
                            firstPolygonContour.list.insertAt(secondPolygonEdge.first, i + 1);
                            secondPolygonContour.list.insertAt(firstPolygonEdge.first, j + 1);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.first);
                            this->_specialIntersections.push_back(secondPolygonEdge.first);
                        } else if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                                 this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon) {
                            // Polygons have one vertex at same positions

                            // Update edges in both polygons
                            firstPolygonContour.list.insertAt(secondPolygonEdge.second, i + 1);
                            secondPolygonContour.list.insertAt(firstPolygonEdge.first, j + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.first);
                            this->_specialIntersections.push_back(secondPolygonEdge.second);
                        } else if (glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                                 this->_vertices.at(secondPolygonEdge.first)) <= this->_epsilon) {
                            // Polygons have one vertex at same positions

                            // Update edges in both polygons
                            firstPolygonContour.list.insertAt(secondPolygonEdge.first, i + 2);
                            secondPolygonContour.list.insertAt(firstPolygonEdge.second, j + 1);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.second);
                            this->_specialIntersections.push_back(secondPolygonEdge.first);
                        } else if (glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                                 this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon) {
                            // Polygons have one vertex at same positions

                            // Update edges in both polygons
                            firstPolygonContour.list.insertAt(secondPolygonEdge.second, i + 2);
                            secondPolygonContour.list.insertAt(firstPolygonEdge.second, j + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.second);
                            this->_specialIntersections.push_back(secondPolygonEdge.second);
                        } else if (glm::distance(intersection, this->_vertices.at(firstPolygonEdge.first)) <=
                                   this->_epsilon) {
                            // Edge of second polygon intersects first vertex of first polygon

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Upadate first polygon
                            firstPolygonContour.list.insertAt(vertex, i);

                            // Upadate second polygon
                            secondPolygonContour.list.insertAt(firstPolygonEdge.first, j + 1);
                            secondPolygonContour.list.insertAt(vertex, j + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.first);
                            this->_specialIntersections.push_back(vertex);
                        } else if (glm::distance(intersection, this->_vertices.at(firstPolygonEdge.second)) <=
                                   this->_epsilon) {
                            // Edge of second polygon intersects second vertex of first polygon

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Upadate first polygon
                            firstPolygonContour.list.insertAt(vertex, i + 1);

                            // Upadate second polygon
                            secondPolygonContour.list.insertAt(firstPolygonEdge.second, j + 1);
                            secondPolygonContour.list.insertAt(vertex, j + 2);

                            // Insert special intersections
                            this->_specialIntersections.push_back(firstPolygonEdge.second);
                            this->_specialIntersections.push_back(vertex);
                        } else if (glm::distance(intersection, this->_vertices.at(secondPolygonEdge.first)) <=
                                   this->_epsilon) {
                            // Edge of first polygon intersects first vertex of second polygon

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Upadate first polygon
                            firstPolygonContour.list.insertAt(secondPolygonEdge.first, i + 1);
                            firstPolygonContour.list.insertAt(vertex, i + 2);

                            // Upadate second polygon
                            secondPolygonContour.list.insertAt(vertex, j);

                            // Insert special intersections
                            this->_specialIntersections.push_back(secondPolygonEdge.first);
                            this->_specialIntersections.push_back(vertex);
                        } else if (glm::distance(intersection, this->_vertices.at(secondPolygonEdge.second)) <=
                                   this->_epsilon) {
                            // Edge of first polygon intersects second vertex of second polygon

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Upadate first polygon
                            firstPolygonContour.list.insertAt(secondPolygonEdge.second, i + 1);
                            firstPolygonContour.list.insertAt(vertex, i + 2);

                            // Upadate second polygon
                            secondPolygonContour.list.insertAt(vertex, j + 1);

                            // Insert special intersections
                            this->_specialIntersections.push_back(secondPolygonEdge.second);
                            this->_specialIntersections.push_back(vertex);
                        } else {
                            // Normal intersection

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Upadate both polygons
                            firstPolygonContour.list.insertAt(vertex, i + 1);
                            secondPolygonContour.list.insertAt(vertex, j + 1);

                            // Insert normal intersection
                            this->_normalIntersections.push_back(vertex);
                        }

                        firstPolygonEdge = Edge{firstPolygonContour.list.getAt(i)->value,
                                                firstPolygonContour.list.getAt(i)->next->value};
                    }
                }
            }
        }
    }
}

bool PolygonOperator::_intersect(Edge first, Edge second, glm::vec2 &intersection) {
    double x1 = this->_vertices.at(first.first).x;
    double y1 = this->_vertices.at(first.first).y;
    double x2 = this->_vertices.at(first.second).x;
    double y2 = this->_vertices.at(first.second).y;
    double x3 = this->_vertices.at(second.first).x;
    double y3 = this->_vertices.at(second.first).y;
    double x4 = this->_vertices.at(second.second).x;
    double y4 = this->_vertices.at(second.second).y;

    double det1 = this->_determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double det2 = this->_determinant(x1 - x3, y1 - y3, x3 - x4, y3 - y4);

    // Check if edges are parallel
    if (std::fabs(det1) < this->_epsilon || std::fabs(det2) < this->_epsilon) {
        return false;
    }

    intersection.x =
        this->_determinant(this->_determinant(x1, y1, x2, y2), x1 - x2, this->_determinant(x3, y3, x4, y4), x3 - x4) /
        det1;
    intersection.y =
        this->_determinant(this->_determinant(x1, y1, x2, y2), y1 - y2, this->_determinant(x3, y3, x4, y4), y3 - y4) /
        det1;

    // Check if intersection is in the boundaries of both edges
    if (((intersection.x >= std::min(x1, x2) && intersection.x <= std::max(x1, x2)) &&
         (intersection.y >= std::min(y1, y2) && intersection.y <= std::max(y1, y2))) &&
        ((intersection.x >= std::min(x3, x4) && intersection.x <= std::max(x3, x4)) &&
         (intersection.y >= std::min(y3, y4) && intersection.y <= std::max(y3, y4)))) {
        return true;
    }

    return false;
}

void PolygonOperator::_walkContours() {
    unsigned int contourIndex = 0;
    uint32_t startVertex = 0;
    bool startingOnNewContour = true;

    if (this->_normalIntersections.size() > 0 || this->_specialIntersections.size() > 0) {
        this->_output.push_back(CircularDLL<uint32_t>{});
    }

    // Process normal intersections
    while (this->_normalIntersections.size() > 0) {
        uint32_t intersectionVertex = this->_normalIntersections.front();
        this->_normalIntersections.pop_front();

        // Set the start of new contour if neccessary
        if (startingOnNewContour) {
            startVertex = intersectionVertex;
            startingOnNewContour = false;
        }

        // Get all edges starting at selected intersection
        std::vector<CircularDLL<uint32_t>::Node *> edges = this->_getEdgesStartingAt(intersectionVertex);
        if (edges.size() == 0) {
            throw std::runtime_error("PolygonOperator::_walkContours(): No edges starting at normal intersection");
        }

        // Select the left-most edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            if (this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value),
                                    this->_vertices.at(edges[selectedEdgeIndex]->next->value),
                                    this->_vertices.at(edges[i]->next->value))) {
                selectedEdgeIndex = i;
            }
        }

        // Mark selected contour as visited
        this->_markContourAsVisited(edges[selectedEdgeIndex]);

        // Process edges starting from selected left-most edge
        uint32_t endVertex = this->_walkUntilIntersectionOrStart(edges[selectedEdgeIndex], contourIndex);
        if (endVertex == startVertex) {
            // Contour is closed
            contourIndex++;
            this->_output.push_back(CircularDLL<uint32_t>{});
            startingOnNewContour = true;
        }
    }

    // Process special intersections
    while (this->_specialIntersections.size() > 0) {
        uint32_t intersectionVertex = this->_specialIntersections.front();
        this->_specialIntersections.pop_front();

        // Set the start of new contour if neccessary
        if (startingOnNewContour) {
            startVertex = intersectionVertex;
            startingOnNewContour = false;
        }

        // Get all edges starting at selected intersection
        std::vector<CircularDLL<uint32_t>::Node *> edges = this->_getEdgesStartingAt(intersectionVertex);
        if (edges.size() == 0) {
            throw std::runtime_error("PolygonOperator::_walkContours(): No edges starting at special intersection");
        }

        // Select the left-most edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            if (this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value),
                                    this->_vertices.at(edges[selectedEdgeIndex]->next->value),
                                    this->_vertices.at(edges[i]->next->value))) {
                selectedEdgeIndex = i;
            }
        }

        // Mark selected contour as visited
        this->_markContourAsVisited(edges[selectedEdgeIndex]);

        // Process edges starting from selected left-most edge
        uint32_t endVertex = this->_walkUntilIntersectionOrStart(edges[selectedEdgeIndex], contourIndex);
        if (endVertex == startVertex) {
            // Contour is closed
            contourIndex++;
            this->_output.push_back(CircularDLL<uint32_t>{});
            startingOnNewContour = true;
        }
    }

    // Add unvisited contours from first polygon to ouput
    for (const Contour &contour : this->_first) {
        if (!contour.visited) {
            this->_output.push_back(contour.list);
        }
    }

    // Add unvisited contours from second polygon to ouput
    for (const Contour &contour : this->_second) {
        if (!contour.visited) {
            this->_output.push_back(contour.list);
        }
    }
}

uint32_t PolygonOperator::_walkUntilIntersectionOrStart(CircularDLL<uint32_t>::Node *start, unsigned int contourIndex) {
    // Index of starting vertex
    uint32_t startVertex = start->value;

    // Process first vertex
    this->_output[contourIndex].insertLast(start->value);
    start = start->next;

    while (std::find(this->_normalIntersections.begin(), this->_normalIntersections.end(), start->value) ==
               this->_normalIntersections.end() &&
           std::find(this->_specialIntersections.begin(), this->_specialIntersections.end(), start->value) ==
               this->_specialIntersections.end() &&
           start->value != startVertex) {
        // Move to next vertex
        this->_output[contourIndex].insertLast(start->value);
        start = start->next;
    }

    return start->value;
}

void PolygonOperator::_markContourAsVisited(CircularDLL<uint32_t>::Node *vertex) {
    // Search in first polygon
    for (Contour &contour : this->_first) {
        for (unsigned int i = 0; i < contour.list.size(); i++) {
            if (vertex == contour.list.getAt(i)) {
                // Found contour including vertex
                contour.visited = true;
                return;
            }
        }
    }

    // Search in second polygon
    for (Contour &contour : this->_second) {
        for (unsigned int i = 0; i < contour.list.size(); i++) {
            if (vertex == contour.list.getAt(i)) {
                // Found contour including vertex
                contour.visited = true;
                return;
            }
        }
    }
}

std::vector<CircularDLL<uint32_t>::Node *> PolygonOperator::_getEdgesStartingAt(uint32_t vertex) {
    std::vector<CircularDLL<uint32_t>::Node *> edges;

    // Search in first polygon
    for (Contour &contour : this->_first) {
        CircularDLL<uint32_t>::Node *edge = contour.list.getValue(vertex);
        if (edge != nullptr) {
            edges.push_back(edge);
        }
    }

    // Search in second polygon
    for (Contour &contour : this->_second) {
        CircularDLL<uint32_t>::Node *edge = contour.list.getValue(vertex);
        if (edge != nullptr) {
            edges.push_back(edge);
        }
    }

    return edges;
}

bool PolygonOperator::_isOnLeftSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point) {
    float a = lineEndingPoint.y - lineStartingPoint.y;
    float b = lineStartingPoint.x - lineEndingPoint.x;
    float c = lineEndingPoint.x * lineStartingPoint.y - lineStartingPoint.x * lineEndingPoint.y;
    float d = a * point.x + b * point.y + c;

    return d < 0;
}

double PolygonOperator::_determinant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

bool PolygonOperator::_isEdgeOnEdge(Edge first, Edge second) {
    return this->_isPointOnEdge(this->_vertices.at(first.first), second) &&
           this->_isPointOnEdge(this->_vertices.at(first.second), second);
}

bool PolygonOperator::_isPointOnEdge(glm::vec2 point, Edge edge) {
    // Check if point lies in the bounding box of edge
    if (point.x < std::min(this->_vertices.at(edge.first).x, this->_vertices.at(edge.second).x) ||
        point.x > std::max(this->_vertices.at(edge.first).x, this->_vertices.at(edge.second).x) ||
        point.y < std::min(this->_vertices.at(edge.first).y, this->_vertices.at(edge.second).y) ||
        point.y > std::max(this->_vertices.at(edge.first).y, this->_vertices.at(edge.second).y)) {
        return false;
    }

    glm::vec2 pointVector = point - this->_vertices.at(edge.second);
    glm::vec2 lineVector = this->_vertices.at(edge.second) - this->_vertices.at(edge.first);

    return glm::abs(glm::cross(glm::vec3{lineVector, 0.f}, glm::vec3{pointVector, 0.f}).z) < this->_epsilon;
}

void PolygonOperator::setEpsilon(double epsilon) {
    if (epsilon < 0) {
        throw std::invalid_argument("PolygonOperator::setEpsilon(): Epsilon must be positive");
    }

    this->_epsilon = epsilon;
}

std::vector<CircularDLL<uint32_t>> PolygonOperator::getPolygon() {
    return this->_output;
}

std::vector<glm::vec2> PolygonOperator::getVertices() {
    return this->_vertices;
}

}  // namespace vft
