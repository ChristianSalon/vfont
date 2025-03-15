/**
 * @file polygon_operator.cpp
 * @author Christian Saloň
 */

#include <iostream>

#include "polygon_operator.h"

namespace vft {

void PolygonOperator::join(const std::vector<glm::vec2> &vertices,
                           const std::vector<CircularDLL<Edge>> &first,
                           const std::vector<CircularDLL<Edge>> &second) {
    this->_initializeContours(vertices, first, second);
    this->_resolveOverlappingEdges();
    this->_resolveIntersectingEdges();

    std::vector<Contour> allContours = this->_first;
    allContours.insert(allContours.end(), this->_second.begin(), this->_second.end());
    this->_removeUnwantedIntersections(this->_intersections, allContours);

    this->_walkContours();
}

void PolygonOperator::_initializeContours(const std::vector<glm::vec2> &vertices,
                                          const std::vector<CircularDLL<Edge>> &first,
                                          const std::vector<CircularDLL<Edge>> &second) {
    // Reset polygons
    this->_first.clear();
    this->_second.clear();
    this->_output.clear();

    // Initialize both polygons
    this->_vertices = vertices;
    for (CircularDLL<Edge> contour : first) {
        for (CircularDLL<Edge> contourWithNoIntersections : this->_resolveSelfIntersections(contour)) {
            this->_first.push_back(Contour{false, contourWithNoIntersections});
        }
    }
    for (CircularDLL<Edge> contour : second) {
        for (CircularDLL<Edge> contourWithNoIntersections : this->_resolveSelfIntersections(contour)) {
            this->_second.push_back(Contour{false, contourWithNoIntersections});
        }
    }
}

std::vector<CircularDLL<Edge>> PolygonOperator::_resolveSelfIntersections(CircularDLL<Edge> contour) {
    std::list<uint32_t> intersections;

    // Handle overlapping edges
    for (unsigned int i = 0; i < contour.size(); i++) {
        Edge firstEdge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};

        for (unsigned int j = i + 2; j < contour.size(); j++) {
            Edge secondEdge{contour.getAt(j)->value.first, contour.getAt(j)->value.second};

            if (glm::distance(this->_vertices.at(firstEdge.first), this->_vertices.at(secondEdge.second)) <=
                    this->_epsilon &&
                glm::distance(this->_vertices.at(firstEdge.second), this->_vertices.at(secondEdge.first)) <=
                    this->_epsilon) {
                // Fully overlapped edges (inverse edges)
                // First edge is A -> B, second is B -> A

                // Delete both edges
                contour.deleteAt(j);
                contour.deleteAt(i);

                // Insert both vertices as intersections
                this->_addIntersectionIfNeeded(intersections, firstEdge.first);
                this->_addIntersectionIfNeeded(intersections, firstEdge.second);

                // Make sure the next edge is the one after deleted edge
                j--;
                // Update first edge to make sure its the next edge after deleted edge
                firstEdge = Edge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};
            } else if (this->_isEdgeOnEdge(secondEdge, firstEdge)) {
                // Second edge fully lies on first edge

                // Update edges so that overlapped part is deleted
                contour.insertAt(Edge{secondEdge.first, firstEdge.second}, i + 1);
                contour.getAt(i)->value.second = secondEdge.second;

                // Because new edge was inserted, increment j
                j++;
                // Delete overlapping edge
                contour.deleteAt(j);

                // Insert vertices of shorter edge as intersections
                this->_addIntersectionIfNeeded(intersections, secondEdge.second);
                this->_addIntersectionIfNeeded(intersections, secondEdge.first);

                // Make sure the next edge is the one after deleted edge
                j--;
                // Update first edge, it should be the first segment of splitted edge
                firstEdge = Edge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};
            } else if (this->_isEdgeOnEdge(firstEdge, secondEdge)) {
                // First edge fully lies on second edge

                // Update edges so that overlapped part is deleted
                contour.insertAt(Edge{firstEdge.first, secondEdge.second}, j + 1);
                contour.getAt(j)->value.second = firstEdge.second;

                // Delete overlapping edge
                contour.deleteAt(i);

                // Insert vertices of shorter edge as intersections
                this->_addIntersectionIfNeeded(intersections, firstEdge.second);
                this->_addIntersectionIfNeeded(intersections, firstEdge.first);

                // First edge was deleted, start comparing edge after deleted edge
                i--;
                break;
            }
        }
    }

    // Handle edges with shared vertices
    for (unsigned int i = 0; i < contour.size(); i++) {
        Edge firstEdge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};

        for (unsigned int j = i + 2; j < contour.size(); j++) {
            Edge secondEdge{contour.getAt(j)->value.first, contour.getAt(j)->value.second};

            if (glm::distance(this->_vertices.at(firstEdge.second), this->_vertices.at(secondEdge.second)) <=
                this->_epsilon) {
                // Edges share end vertex

                // TODO: Insert shared vertex as intersection ?
                // this->_addIntersectionIfNeeded(intersections, firstEdge.second);
            }
        }
    }

    // Handle normal intersections
    for (unsigned int i = 0; i < contour.size(); i++) {
        Edge firstEdge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};

        for (unsigned int j = i + 2; j < contour.size(); j++) {
            Edge secondEdge{contour.getAt(j)->value.first, contour.getAt(j)->value.second};

            glm::vec2 intersection{0, 0};
            if (this->_intersect(firstEdge, secondEdge, intersection)) {
                if (glm::distance(this->_vertices.at(firstEdge.first), this->_vertices.at(secondEdge.first)) >
                        this->_epsilon &&
                    glm::distance(this->_vertices.at(firstEdge.first), this->_vertices.at(secondEdge.second)) >
                        this->_epsilon &&
                    glm::distance(this->_vertices.at(firstEdge.second), this->_vertices.at(secondEdge.first)) >
                        this->_epsilon &&
                    glm::distance(this->_vertices.at(firstEdge.second), this->_vertices.at(secondEdge.second)) >
                        this->_epsilon) {
                    // Normal intersection

                    // Add intersection to vertices
                    uint32_t vertex = this->_vertices.size();
                    this->_vertices.push_back(intersection);

                    // Update first edge
                    contour.insertAt(Edge{vertex, firstEdge.second}, i + 1);
                    contour.getAt(i)->value.second = vertex;

                    // Because new edge was inserted, increment j
                    j++;

                    // Update second edge
                    contour.insertAt(Edge{vertex, secondEdge.second}, j + 1);
                    contour.getAt(j)->value.second = vertex;

                    // Insert intersection
                    this->_addIntersectionIfNeeded(intersections, vertex);

                    firstEdge = Edge{contour.getAt(i)->value.first, contour.getAt(i)->value.second};
                }
            }
        }
    }

    this->_removeUnwantedIntersections(intersections, {Contour{false, contour}});

    if (intersections.size() == 0) {
        return {contour};
    }

    // Walk contours
    uint32_t startVertex = 0;  // Starting vertex of current contour
    uint32_t endVertex = 0;    // Last processed vertex of current contour
    unsigned int contourIndex = 0;
    std::vector<CircularDLL<Edge>> output;

    // Process all intersections
    while (intersections.size() > 0) {
        uint32_t intersectionVertex;  // Vertex from which to start traversing contour

        if (startVertex == endVertex) {
            // Start processing new contour
            intersectionVertex = intersections.front();
            intersections.pop_front();

            startVertex = intersectionVertex;

            output.push_back(CircularDLL<Edge>{});
        } else {
            // Contour is not yet fully processed (closed)
            intersectionVertex = endVertex;
            intersections.remove(intersectionVertex);
        }

        // Get all edges starting at selected intersection
        std::vector<CircularDLL<Edge>::Node *> edges;
        CircularDLL<Edge>::Node *edge = contour.getFirst();
        for (unsigned int i = 0; i < contour.size(); i++) {
            if (edge->value.first == intersectionVertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }

        if (edges.size() == 0) {
            throw std::runtime_error("PolygonOperator::_resolveSelfIntersections(): No edges starting at intersection");
        }

        // Select the left-most edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            if (this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value.first),
                                    this->_vertices.at(edges[selectedEdgeIndex]->value.second),
                                    this->_vertices.at(edges[i]->value.second))) {
                selectedEdgeIndex = i;
            }
        }

        // Process edges starting from selected left-most edge
        CircularDLL<Edge>::Node *startEdge = edges[selectedEdgeIndex];

        // Process edges until intersection or starting vertex
        while (std::find(intersections.begin(), intersections.end(), startEdge->value.second) ==
                   intersections.end()  // End of current edge is a intersection
               && (output[contourIndex].size() == 0 ||
                   startEdge->value.second !=
                       output[contourIndex].getFirst()->value.first)  // End of edge is contour start (closed contour)
        ) {
            // Move to next edge
            output[contourIndex].insertLast(startEdge->value);
            startEdge = startEdge->next;
        }
        // Add edge ending at intersection or start to output
        output[contourIndex].insertLast(startEdge->value);

        endVertex = startEdge->value.second;
        if (endVertex == startVertex) {
            // Contour is closed
            contourIndex++;
        }
    }

    return output;
}

void PolygonOperator::_resolveOverlappingEdges() {
    for (Contour &firstPolygonContour : this->_first) {
        for (unsigned int i = 0; i < firstPolygonContour.list.size(); i++) {
            Edge firstPolygonEdge{firstPolygonContour.list.getAt(i)->value.first,
                                  firstPolygonContour.list.getAt(i)->value.second};
            bool wasFirstPolygonEdgeChanged = false;

            for (Contour &secondPolygonContour : this->_second) {
                for (unsigned int j = 0; j < secondPolygonContour.list.size(); j++) {
                    Edge secondPolygonEdge{secondPolygonContour.list.getAt(j)->value.first,
                                           secondPolygonContour.list.getAt(j)->value.second};

                    if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                      this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon &&
                        glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                      this->_vertices.at(secondPolygonEdge.first)) <= this->_epsilon) {
                        // Fully overlapped edges (inverse edges)
                        // First edge is A -> B, second is B -> A

                        // Delete both edges
                        firstPolygonContour.list.deleteAt(i);
                        secondPolygonContour.list.deleteAt(j);

                        // Insert vertices of overlapped edges as intersections
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.first);
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.second);

                        i--;  // Decrement because edge from first polygon was deleted
                        wasFirstPolygonEdgeChanged = true;
                        break;
                    } else if (this->_isEdgeOnEdge(secondPolygonEdge, firstPolygonEdge)) {
                        // Second edge fully lies on first edge

                        // Update edges in first polygon so that overlapped part is deleted
                        firstPolygonContour.list.insertAt(Edge{secondPolygonEdge.first, firstPolygonEdge.second},
                                                          i + 1);
                        firstPolygonContour.list.getAt(i)->value.second = secondPolygonEdge.second;

                        // Delete overlapping edge in second polygon
                        secondPolygonContour.list.deleteAt(j);

                        // Insert vertices of shorter edge as intersections
                        this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.second);
                        this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.first);

                        wasFirstPolygonEdgeChanged = true;
                        break;
                    } else if (this->_isEdgeOnEdge(firstPolygonEdge, secondPolygonEdge)) {
                        // First edge fully lies on second edge

                        // Update edges in second polygon so that overlapped part is deleted
                        secondPolygonContour.list.insertAt(Edge{firstPolygonEdge.first, secondPolygonEdge.second},
                                                           j + 1);
                        secondPolygonContour.list.getAt(j)->value.second = firstPolygonEdge.second;

                        // Delete overlapping edge in second polygon
                        firstPolygonContour.list.deleteAt(i);

                        // Insert vertices of shorter edge as intersections
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.second);
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.first);

                        i--;  // Decrement because edge from first polygon was deleted
                        wasFirstPolygonEdgeChanged = true;
                        break;
                    }
                }

                if (wasFirstPolygonEdgeChanged) {
                    break;
                }
            }
        }
    }
}

void PolygonOperator::_resolveIntersectingEdges() {
    for (Contour &firstPolygonContour : this->_first) {
        for (unsigned int i = 0; i < firstPolygonContour.list.size(); i++) {
            Edge firstPolygonEdge{firstPolygonContour.list.getAt(i)->value.first,
                                  firstPolygonContour.list.getAt(i)->value.second};
            bool wasFirstPolygonEdgeChanged = false;

            for (Contour &secondPolygonContour : this->_second) {
                for (unsigned int j = 0; j < secondPolygonContour.list.size(); j++) {
                    Edge secondPolygonEdge{secondPolygonContour.list.getAt(j)->value.first,
                                           secondPolygonContour.list.getAt(j)->value.second};

                    // Check if edges intersect
                    glm::vec2 intersection{0, 0};
                    if (this->_intersect(firstPolygonEdge, secondPolygonEdge, intersection)) {
                        if (glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                          this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon) {
                            // Polygons have one vertex at same positions

                            // Insert shared vertex as intersection
                            this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.second);
                        } else if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                                 this->_vertices.at(secondPolygonEdge.first)) > this->_epsilon &&
                                   glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                                 this->_vertices.at(secondPolygonEdge.second)) > this->_epsilon &&
                                   glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                                 this->_vertices.at(secondPolygonEdge.first)) > this->_epsilon) {
                            // Normal intersection

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Update first polygon
                            firstPolygonContour.list.getAt(i)->value.second = vertex;
                            firstPolygonContour.list.insertAt(Edge{vertex, firstPolygonEdge.second}, i + 1);

                            // Update second polygon
                            secondPolygonContour.list.getAt(j)->value.second = vertex;
                            secondPolygonContour.list.insertAt(Edge{vertex, secondPolygonEdge.second}, j + 1);

                            // Insert intersection
                            this->_addIntersectionIfNeeded(this->_intersections, vertex);

                            wasFirstPolygonEdgeChanged = true;
                            break;
                        }
                    }
                }

                if (wasFirstPolygonEdgeChanged) {
                    // Make sure that the next selected edge is the first one of changed edges
                    // In some cases there may be another intersection
                    i--;
                    break;
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
    uint32_t startVertex = 0;  // Starting vertex of current contour
    uint32_t endVertex = 0;    // Last processed vertex of current contour
    unsigned int contourIndex = 0;

    // Process intersections
    while (this->_intersections.size() > 0) {
        uint32_t intersectionVertex;  // Vertex from which to start traversing contour

        if (startVertex == endVertex) {
            // Start processing new contour
            intersectionVertex = this->_intersections.front();
            this->_intersections.pop_front();

            startVertex = intersectionVertex;

            this->_output.push_back(CircularDLL<Edge>{});
        } else {
            // Contour is not yet fully processed (closed)
            intersectionVertex = endVertex;
            this->_intersections.remove(intersectionVertex);
        }

        // Get all edges starting at selected intersection
        std::vector<CircularDLL<Edge>::Node *> edges = this->_getEdgesStartingAt(intersectionVertex);
        if (edges.size() == 0) {
            throw std::runtime_error("PolygonOperator::_walkContours(): No edges starting at intersection");
        }

        // Select the left-most edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            if (this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value.first),
                                    this->_vertices.at(edges[selectedEdgeIndex]->value.second),
                                    this->_vertices.at(edges[i]->value.second))) {
                selectedEdgeIndex = i;
            }
        }

        // Mark selected contour as visited
        this->_markContourAsVisited(edges[selectedEdgeIndex]);

        // Process edges starting from selected left-most edge
        endVertex = this->_walkUntilIntersectionOrStart(edges[selectedEdgeIndex], contourIndex);
        if (endVertex == startVertex) {
            // Contour is closed
            contourIndex++;
        }
    }

    // Add unvisited contours from first polygon to ouput
    for (const Contour &contour : this->_first) {
        if (!contour.visited && contour.list.size() > 0) {
            this->_output.push_back(contour.list);
        }
    }

    // Add unvisited contours from second polygon to ouput
    for (const Contour &contour : this->_second) {
        if (!contour.visited && contour.list.size() > 0) {
            this->_output.push_back(contour.list);
        }
    }
}

uint32_t PolygonOperator::_walkUntilIntersectionOrStart(CircularDLL<Edge>::Node *start, unsigned int contourIndex) {
    while (std::find(this->_intersections.begin(), this->_intersections.end(), start->value.second) ==
               this->_intersections.end()  // End of current edge is a intersection
           &&
           (this->_output[contourIndex].size() == 0 ||
            start->value.second !=
                this->_output[contourIndex].getFirst()->value.first)  // End of edge is contour start (closed contour)
    ) {
        // Move to next edge
        this->_output[contourIndex].insertLast(start->value);
        start = start->next;
    }

    // Add edge ending at intersection or start to output
    this->_output[contourIndex].insertLast(start->value);

    return start->value.second;
}

void PolygonOperator::_markContourAsVisited(CircularDLL<Edge>::Node *vertex) {
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

void PolygonOperator::_addIntersectionIfNeeded(std::list<uint32_t> &intersections, uint32_t intersection) {
    if (std::find(intersections.begin(), intersections.end(), intersection) == intersections.end()) {
        intersections.push_back(intersection);
    }
}

void PolygonOperator::_removeUnwantedIntersections(std::list<uint32_t> &intersections,
                                                   const std::vector<Contour> &contours) {
    std::erase_if(intersections, [&](uint32_t intersection) {
        for (const Contour &contour : contours) {
            for (unsigned int i = 0; i < contour.list.size(); i++) {
                if (contour.list.getAt(i)->value.first == intersection ||
                    contour.list.getAt(i)->value.second == intersection) {
                    return false;  // Valid intersection
                }
            }
        }

        return true;  // No edges starting or ending at intersection
    });
}

std::vector<CircularDLL<Edge>::Node *> PolygonOperator::_getEdgesStartingAt(uint32_t vertex) {
    std::vector<CircularDLL<Edge>::Node *> edges;

    // Search in first polygon
    for (Contour &contour : this->_first) {
        CircularDLL<Edge>::Node *edge = contour.list.getFirst();
        for (unsigned int i = 0; i < contour.list.size(); i++) {
            if (edge->value.first == vertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }
    }

    // Search in second polygon
    for (Contour &contour : this->_second) {
        CircularDLL<Edge>::Node *edge = contour.list.getFirst();
        for (unsigned int i = 0; i < contour.list.size(); i++) {
            if (edge->value.first == vertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
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

std::vector<CircularDLL<Edge>> PolygonOperator::getPolygon() {
    return this->_output;
}

std::vector<glm::vec2> PolygonOperator::getVertices() {
    return this->_vertices;
}

}  // namespace vft
