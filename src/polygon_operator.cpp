/**
 * @file polygon_operator.cpp
 * @author Christian Saloň
 */

#include "polygon_operator.h"

namespace vft {

/**
 * @brief Union of two polygons
 *
 * @param vertices Vertices of both polygons
 * @param first First polygon
 * @param second Second polygon
 */
void PolygonOperator::join(const std::vector<glm::vec2> &vertices,
                           const std::vector<Outline> &first,
                           const std::vector<Outline> &second) {
    this->_initializeContours(vertices, first, second);
    this->_resolveOverlappingEdges();
    this->_resolveIntersectingEdges();

    std::vector<Contour> allContours = this->_first;
    allContours.insert(allContours.end(), this->_second.begin(), this->_second.end());
    this->_removeUnwantedIntersections(this->_intersections, allContours);

    this->_walkContours();
}

/**
 * @brief Initialize polygons to be ready for traversal
 *
 * @param vertices Vertices of both polygons
 * @param first First polygon
 * @param second Second polygon
 */
void PolygonOperator::_initializeContours(const std::vector<glm::vec2> &vertices,
                                          const std::vector<Outline> &first,
                                          const std::vector<Outline> &second) {
    // Reset polygons
    this->_first.clear();
    this->_second.clear();
    this->_output.clear();

    // Initialize first polygon
    this->_vertices = vertices;
    for (Outline outline : first) {
        for (Outline contourWithNoIntersections : this->_resolveSelfIntersections(outline)) {
            this->_first.push_back(Contour{false, contourWithNoIntersections});
        }
    }

    // Initialize second polygon
    for (Outline outline : second) {
        for (Outline contourWithNoIntersections : this->_resolveSelfIntersections(outline)) {
            this->_second.push_back(Contour{false, contourWithNoIntersections});
        }
    }
}

/**
 * @brief Resolve self intersections of one outline
 *
 * @param outline Given outline
 *
 * @return Outlines with no self intersections
 */
std::vector<Outline> PolygonOperator::_resolveSelfIntersections(Outline outline) {
    std::list<uint32_t> intersections;

    // Handle overlapping edges
    for (unsigned int i = 0; i < outline.edges.size(); i++) {
        Edge firstEdge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};

        for (unsigned int j = i + 2; j < outline.edges.size(); j++) {
            Edge secondEdge{outline.edges.getAt(j)->value.first, outline.edges.getAt(j)->value.second};

            // Skip duplicate edges
            if (firstEdge.first == secondEdge.first && firstEdge.second == secondEdge.second) {
                continue;
            }

            if (glm::distance(this->_vertices.at(firstEdge.first), this->_vertices.at(secondEdge.second)) <=
                    this->_epsilon &&
                glm::distance(this->_vertices.at(firstEdge.second), this->_vertices.at(secondEdge.first)) <=
                    this->_epsilon) {
                // Fully overlapped edges (inverse edges)
                // First edge is A -> B, second is B -> A

                // Delete both edges
                outline.edges.deleteAt(j);
                outline.edges.deleteAt(i);

                // Insert both vertices as intersections
                this->_addIntersectionIfNeeded(intersections, firstEdge.first);
                this->_addIntersectionIfNeeded(intersections, firstEdge.second);

                // Make sure the next edge is the one after deleted edge
                j--;
                // Update first edge to make sure its the next edge after deleted edge
                firstEdge = Edge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};
            } else if (this->_isEdgeOnEdge(secondEdge, firstEdge)) {
                // Second edge fully lies on first edge

                // Update edges so that overlapped part is deleted
                outline.edges.insertAt(Edge{secondEdge.first, firstEdge.second}, i + 1);
                outline.edges.getAt(i)->value.second = secondEdge.second;

                // Because new edge was inserted, increment j
                j++;
                // Delete overlapping edge
                outline.edges.deleteAt(j);

                // Insert vertices of shorter edge as intersections
                this->_addIntersectionIfNeeded(intersections, secondEdge.second);
                this->_addIntersectionIfNeeded(intersections, secondEdge.first);

                // Make sure the next edge is the one after deleted edge
                j--;
                // Update first edge, it should be the first segment of splitted edge
                firstEdge = Edge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};
            } else if (this->_isEdgeOnEdge(firstEdge, secondEdge)) {
                // First edge fully lies on second edge

                // Update edges so that overlapped part is deleted
                outline.edges.insertAt(Edge{firstEdge.first, secondEdge.second}, j + 1);
                outline.edges.getAt(j)->value.second = firstEdge.second;

                // Delete overlapping edge
                outline.edges.deleteAt(i);

                // Insert vertices of shorter edge as intersections
                this->_addIntersectionIfNeeded(intersections, firstEdge.second);
                this->_addIntersectionIfNeeded(intersections, firstEdge.first);

                // First edge was deleted, start comparing edge after deleted edge
                i--;
                break;
            }
        }
    }

    // Handle normal intersections
    for (unsigned int i = 0; i < outline.edges.size(); i++) {
        Edge firstEdge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};

        for (unsigned int j = i + 2; j < outline.edges.size(); j++) {
            Edge secondEdge{outline.edges.getAt(j)->value.first, outline.edges.getAt(j)->value.second};

            // Skip edges that share at least one vertex
            if (firstEdge.first == secondEdge.first || firstEdge.first == secondEdge.second ||
                firstEdge.second == secondEdge.first || firstEdge.second == secondEdge.second) {
                continue;
            }

            glm::vec2 intersection{0, 0};
            if (this->_intersect(firstEdge, secondEdge, intersection)) {
                if (glm::distance(intersection, this->_vertices.at(firstEdge.first)) <= this->_epsilon) {
                    // Polygons have intersection at start vertex of first edge

                    // Insert shared vertex as intersection
                    this->_addIntersectionIfNeeded(intersections, firstEdge.first);

                    // Update second polygon if neccessary
                    if (secondEdge.first != firstEdge.first && secondEdge.second != firstEdge.first) {
                        outline.edges.getAt(j)->value.second = firstEdge.first;
                        outline.edges.insertAt(Edge{firstEdge.first, secondEdge.second}, j + 1);
                    }
                } else if (glm::distance(intersection, this->_vertices.at(firstEdge.second)) <= this->_epsilon) {
                    // Polygons have intersection at end vertex of first edge

                    // Insert shared vertex as intersection
                    this->_addIntersectionIfNeeded(intersections, firstEdge.second);

                    // Update second polygon if neccessary
                    if (secondEdge.first != firstEdge.second && secondEdge.second != firstEdge.second) {
                        outline.edges.getAt(j)->value.second = firstEdge.second;
                        outline.edges.insertAt(Edge{firstEdge.second, secondEdge.second}, j + 1);
                    }
                } else if (glm::distance(intersection, this->_vertices.at(secondEdge.first)) <= this->_epsilon) {
                    // Polygons have intersection at start vertex of second edge

                    // Insert shared vertex as intersection
                    this->_addIntersectionIfNeeded(intersections, secondEdge.first);

                    // Update first polygon if neccessary
                    if (firstEdge.first != secondEdge.first && firstEdge.second != secondEdge.first) {
                        outline.edges.getAt(i)->value.second = secondEdge.first;
                        outline.edges.insertAt(Edge{secondEdge.first, firstEdge.second}, i + 1);

                        firstEdge = Edge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};
                    }
                } else if (glm::distance(intersection, this->_vertices.at(secondEdge.second)) <= this->_epsilon) {
                    // Polygons have intersection at end vertex of second edge

                    // Insert shared vertex as intersection
                    this->_addIntersectionIfNeeded(intersections, secondEdge.second);

                    // Update first polygon if neccessary
                    if (firstEdge.first != secondEdge.second && firstEdge.second != secondEdge.second) {
                        outline.edges.getAt(i)->value.second = secondEdge.second;
                        outline.edges.insertAt(Edge{secondEdge.second, firstEdge.second}, i + 1);

                        firstEdge = Edge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};
                    }
                } else {
                    // Normal intersection

                    // Add intersection to vertices
                    uint32_t vertex = this->_vertices.size();
                    this->_vertices.push_back(intersection);

                    // Update first edge
                    outline.edges.insertAt(Edge{vertex, firstEdge.second}, i + 1);
                    outline.edges.getAt(i)->value.second = vertex;

                    // Because new edge was inserted, increment j
                    j++;

                    // Update second edge
                    outline.edges.insertAt(Edge{vertex, secondEdge.second}, j + 1);
                    outline.edges.getAt(j)->value.second = vertex;

                    // Insert intersection
                    this->_addIntersectionIfNeeded(intersections, vertex);

                    firstEdge = Edge{outline.edges.getAt(i)->value.first, outline.edges.getAt(i)->value.second};
                }
            }
        }
    }

    this->_removeUnwantedIntersections(intersections, {Contour{false, outline}});

    if (intersections.size() == 0) {
        return {outline};
    }

    // Walk contours
    uint32_t startVertex = 0;  // Starting vertex of current contour
    uint32_t endVertex = 0;    // Last processed vertex of current contour
    unsigned int contourIndex = 0;
    std::vector<Outline> output;
    unsigned int intersectionCount = intersections.size();
    unsigned int processedIntersections = 0;

    // Process all intersections
    while (intersections.size() > 0) {
        uint32_t intersectionVertex;  // Vertex from which to start traversing contour

        if (startVertex == endVertex) {
            // Start processing new contour
            intersectionVertex = intersections.front();
            intersections.pop_front();

            startVertex = intersectionVertex;

            output.push_back(Outline{});
        } else {
            // Contour is not yet fully processed (closed)
            intersectionVertex = endVertex;
            intersections.remove(intersectionVertex);
        }

        // Get all edges starting at selected intersection
        std::vector<CircularDLL<Edge>::Node *> edges;
        CircularDLL<Edge>::Node *edge = outline.edges.getFirst();
        for (unsigned int i = 0; i < outline.edges.size(); i++) {
            if (edge->value.first == intersectionVertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }

        if (edges.size() == 0) {
            throw std::runtime_error("PolygonOperator::_resolveSelfIntersections(): No edges starting at intersection");
        }

        // Select the start edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            bool isOnLeftSide = this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value.first),
                                                    this->_vertices.at(edges[selectedEdgeIndex]->value.second),
                                                    this->_vertices.at(edges[i]->value.second));

            if (this->_getOrientationOfSubcontour(outline, edges[selectedEdgeIndex]) == Outline::Orientation::CW &&
                this->_getOrientationOfSubcontour(outline, edges[i]) == Outline::Orientation::CW) {
                // Both subcontours define a filled area
                if (isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            } else if (this->_getOrientationOfSubcontour(outline, edges[selectedEdgeIndex]) ==
                           Outline::Orientation::CCW &&
                       this->_getOrientationOfSubcontour(outline, edges[i]) == Outline::Orientation::CCW) {
                // Both subcontours define a hole
                if (!isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            } else {
                // Select the left-most edge
                if (isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            }
        }

        // Process edges starting from selected edge
        CircularDLL<Edge>::Node *startEdge = edges[selectedEdgeIndex];

        // Process edges until intersection or starting vertex
        while (
            std::find(intersections.begin(), intersections.end(), startEdge->value.second) ==
                intersections.end()  // End of current edge is a intersection
            &&
            (output[contourIndex].edges.size() == 0 ||
             startEdge->value.second !=
                 output[contourIndex].edges.getFirst()->value.first)  // End of edge is contour start (closed contour)
        ) {
            // Move to next edge
            output[contourIndex].edges.insertLast(startEdge->value);
            startEdge = startEdge->next;
        }
        // Add edge ending at intersection or start to output
        output[contourIndex].edges.insertLast(startEdge->value);

        endVertex = startEdge->value.second;
        processedIntersections++;
        if (endVertex == startVertex) {
            // Contour is closed, set orientation of contour
            output[contourIndex].orientation =
                this->_getOrientationOfSubcontour(output[contourIndex], output[contourIndex].edges.getFirst());
            contourIndex++;
        }
    }

    return output;
}

/**
 * @brief Resolve overlapping edges between the first and second polygon
 */
void PolygonOperator::_resolveOverlappingEdges() {
    for (Contour &firstPolygonContour : this->_first) {
        for (unsigned int i = 0; i < firstPolygonContour.outline.edges.size(); i++) {
            Edge firstPolygonEdge{firstPolygonContour.outline.edges.getAt(i)->value.first,
                                  firstPolygonContour.outline.edges.getAt(i)->value.second};
            bool wasFirstPolygonEdgeChanged = false;

            for (Contour &secondPolygonContour : this->_second) {
                for (unsigned int j = 0; j < secondPolygonContour.outline.edges.size(); j++) {
                    Edge secondPolygonEdge{secondPolygonContour.outline.edges.getAt(j)->value.first,
                                           secondPolygonContour.outline.edges.getAt(j)->value.second};

                    if (glm::distance(this->_vertices.at(firstPolygonEdge.first),
                                      this->_vertices.at(secondPolygonEdge.second)) <= this->_epsilon &&
                        glm::distance(this->_vertices.at(firstPolygonEdge.second),
                                      this->_vertices.at(secondPolygonEdge.first)) <= this->_epsilon) {
                        // Fully overlapped edges (inverse edges)
                        // First edge is A -> B, second is B -> A

                        // Delete both edges
                        firstPolygonContour.outline.edges.deleteAt(i);
                        secondPolygonContour.outline.edges.deleteAt(j);

                        // Insert vertices of overlapped edges as intersections
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.first);
                        this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.second);

                        i--;  // Decrement because edge from first polygon was deleted
                        wasFirstPolygonEdgeChanged = true;
                        break;
                    } else if (this->_isEdgeOnEdge(secondPolygonEdge, firstPolygonEdge)) {
                        // Second edge fully lies on first edge

                        // Update edges in first polygon so that overlapped part is deleted
                        firstPolygonContour.outline.edges.insertAt(
                            Edge{secondPolygonEdge.first, firstPolygonEdge.second}, i + 1);
                        firstPolygonContour.outline.edges.getAt(i)->value.second = secondPolygonEdge.second;

                        // Delete overlapping edge in second polygon
                        secondPolygonContour.outline.edges.deleteAt(j);

                        // Insert vertices of shorter edge as intersections
                        this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.second);
                        this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.first);

                        wasFirstPolygonEdgeChanged = true;
                        break;
                    } else if (this->_isEdgeOnEdge(firstPolygonEdge, secondPolygonEdge)) {
                        // First edge fully lies on second edge

                        // Update edges in second polygon so that overlapped part is deleted
                        secondPolygonContour.outline.edges.insertAt(
                            Edge{firstPolygonEdge.first, secondPolygonEdge.second}, j + 1);
                        secondPolygonContour.outline.edges.getAt(j)->value.second = firstPolygonEdge.second;

                        // Delete overlapping edge in second polygon
                        firstPolygonContour.outline.edges.deleteAt(i);

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

/**
 * @brief Resolve normal intersections and intersections at shared vertex between the first and second polygon
 */
void PolygonOperator::_resolveIntersectingEdges() {
    for (Contour &firstPolygonContour : this->_first) {
        for (unsigned int i = 0; i < firstPolygonContour.outline.edges.size(); i++) {
            Edge firstPolygonEdge{firstPolygonContour.outline.edges.getAt(i)->value.first,
                                  firstPolygonContour.outline.edges.getAt(i)->value.second};
            bool wasFirstPolygonEdgeChanged = false;

            for (Contour &secondPolygonContour : this->_second) {
                for (unsigned int j = 0; j < secondPolygonContour.outline.edges.size(); j++) {
                    Edge secondPolygonEdge{secondPolygonContour.outline.edges.getAt(j)->value.first,
                                           secondPolygonContour.outline.edges.getAt(j)->value.second};

                    // Skip edges that share at least one vertex
                    if (firstPolygonEdge.first == secondPolygonEdge.first ||
                        firstPolygonEdge.first == secondPolygonEdge.second ||
                        firstPolygonEdge.second == secondPolygonEdge.first ||
                        firstPolygonEdge.second == secondPolygonEdge.second) {
                        continue;
                    }

                    // Check if edges intersect
                    glm::vec2 intersection{0, 0};
                    if (this->_intersect(firstPolygonEdge, secondPolygonEdge, intersection)) {
                        if (glm::distance(intersection, this->_vertices.at(firstPolygonEdge.first)) <= this->_epsilon) {
                            // Polygons have intersection at start vertex of first edge

                            // Insert shared vertex as intersection
                            this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.first);

                            // Update second polygon if neccessary
                            if (secondPolygonEdge.first != firstPolygonEdge.first &&
                                secondPolygonEdge.second != firstPolygonEdge.first) {
                                secondPolygonContour.outline.edges.getAt(j)->value.second = firstPolygonEdge.first;
                                secondPolygonContour.outline.edges.insertAt(
                                    Edge{firstPolygonEdge.first, secondPolygonEdge.second}, j + 1);
                            }
                        } else if (glm::distance(intersection, this->_vertices.at(firstPolygonEdge.second)) <=
                                   this->_epsilon) {
                            // Polygons have intersection at end vertex of first edge

                            // Insert shared vertex as intersection
                            this->_addIntersectionIfNeeded(this->_intersections, firstPolygonEdge.second);

                            // Update second polygon if neccessary
                            if (secondPolygonEdge.first != firstPolygonEdge.second &&
                                secondPolygonEdge.second != firstPolygonEdge.second) {
                                secondPolygonContour.outline.edges.getAt(j)->value.second = firstPolygonEdge.second;
                                secondPolygonContour.outline.edges.insertAt(
                                    Edge{firstPolygonEdge.second, secondPolygonEdge.second}, j + 1);
                            }
                        } else if (glm::distance(intersection, this->_vertices.at(secondPolygonEdge.first)) <=
                                   this->_epsilon) {
                            // Polygons have intersection at start vertex of second edge

                            // Insert shared vertex as intersection
                            this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.first);

                            // Update first polygon if neccessary
                            if (firstPolygonEdge.first != secondPolygonEdge.first &&
                                firstPolygonEdge.second != secondPolygonEdge.first) {
                                firstPolygonContour.outline.edges.getAt(i)->value.second = secondPolygonEdge.first;
                                firstPolygonContour.outline.edges.insertAt(
                                    Edge{secondPolygonEdge.first, firstPolygonEdge.second}, i + 1);

                                wasFirstPolygonEdgeChanged = true;
                                break;
                            }
                        } else if (glm::distance(intersection, this->_vertices.at(secondPolygonEdge.second)) <=
                                   this->_epsilon) {
                            // Polygons have intersection at end vertex of second edge

                            // Insert shared vertex as intersection
                            this->_addIntersectionIfNeeded(this->_intersections, secondPolygonEdge.second);

                            // Update first polygon if neccessary
                            if (firstPolygonEdge.first != secondPolygonEdge.second &&
                                firstPolygonEdge.second != secondPolygonEdge.second) {
                                firstPolygonContour.outline.edges.getAt(i)->value.second = secondPolygonEdge.second;
                                firstPolygonContour.outline.edges.insertAt(
                                    Edge{secondPolygonEdge.second, firstPolygonEdge.second}, i + 1);

                                wasFirstPolygonEdgeChanged = true;
                                break;
                            }
                        } else {
                            // Normal intersection

                            // Add intersection to vertices
                            uint32_t vertex = this->_vertices.size();
                            this->_vertices.push_back(intersection);

                            // Update first polygon
                            firstPolygonContour.outline.edges.getAt(i)->value.second = vertex;
                            firstPolygonContour.outline.edges.insertAt(Edge{vertex, firstPolygonEdge.second}, i + 1);

                            // Update second polygon
                            secondPolygonContour.outline.edges.getAt(j)->value.second = vertex;
                            secondPolygonContour.outline.edges.insertAt(Edge{vertex, secondPolygonEdge.second}, j + 1);

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

/**
 * @brief Check if edges intersect
 *
 * @param first First edge
 * @param second Second edge
 * @param intersection Sets the point of intesection if exists
 *
 * @return True if edges intersect
 */
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

/**
 * @brief Traverse both polygons to construct the output polygon
 */
void PolygonOperator::_walkContours() {
    uint32_t startVertex = 0;  // Starting vertex of current contour
    uint32_t endVertex = 0;    // Last processed vertex of current contour
    unsigned int contourIndex = 0;
    unsigned int intersectionCount = this->_intersections.size();
    unsigned int processedIntersections = 0;

    // Process intersections
    while (this->_intersections.size() > 0) {
        uint32_t intersectionVertex;  // Vertex from which to start traversing contour

        if (startVertex == endVertex) {
            // Start processing new contour
            intersectionVertex = this->_intersections.front();
            this->_intersections.pop_front();

            startVertex = intersectionVertex;

            this->_output.push_back(Outline{});
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

        // Select the next edge
        unsigned int selectedEdgeIndex = 0;
        for (unsigned int i = 1; i < edges.size(); i++) {
            bool isOnLeftSide = this->_isOnLeftSide(this->_vertices.at(edges[selectedEdgeIndex]->value.first),
                                                    this->_vertices.at(edges[selectedEdgeIndex]->value.second),
                                                    this->_vertices.at(edges[i]->value.second));
            if (this->_getContourOfEdge(edges[selectedEdgeIndex]).outline.orientation == Outline::Orientation::CW &&
                this->_getContourOfEdge(edges[i]).outline.orientation == Outline::Orientation::CW) {
                // Both contours define a filled area
                if (isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            } else if (this->_getContourOfEdge(edges[selectedEdgeIndex]).outline.orientation ==
                           Outline::Orientation::CCW &&
                       this->_getContourOfEdge(edges[i]).outline.orientation == Outline::Orientation::CCW) {
                // Both contours define a hole
                if (!isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            } else {
                // Select the left-most edge
                if (isOnLeftSide) {
                    selectedEdgeIndex = i;
                }
            }
        }

        // Mark selected contour as visited
        this->_markContourAsVisited(edges[selectedEdgeIndex]);

        // Process edges starting from selected left-most edge
        endVertex = this->_walkUntilIntersectionOrStart(edges[selectedEdgeIndex], contourIndex);
        processedIntersections++;
        if (endVertex == startVertex) {
            // Contour is closed, set orientation of contour
            this->_output[contourIndex].orientation = this->_getOrientationOfSubcontour(
                this->_output[contourIndex], this->_output[contourIndex].edges.getFirst());
            contourIndex++;
        }
    }

    // Add unvisited contours from first polygon to ouput
    for (const Contour &contour : this->_first) {
        if (!contour.visited && contour.outline.edges.size() > 0) {
            this->_output.push_back(contour.outline);
        }
    }

    // Add unvisited contours from second polygon to ouput
    for (const Contour &contour : this->_second) {
        if (!contour.visited && contour.outline.edges.size() > 0) {
            this->_output.push_back(contour.outline);
        }
    }
}

/**
 * @brief Traverse given contour until start of currently processed contour or until intersection
 *
 * @param start Starting edge
 * @param contourIndex Index of currently processed contour
 *
 * @return Index of vertex where traversal ended
 */
uint32_t PolygonOperator::_walkUntilIntersectionOrStart(CircularDLL<Edge>::Node *start, unsigned int contourIndex) {
    while (std::find(this->_intersections.begin(), this->_intersections.end(), start->value.second) ==
               this->_intersections.end()  // End of current edge is a intersection
           &&
           (this->_output[contourIndex].edges.size() == 0 ||
            start->value.second != this->_output[contourIndex].edges.getFirst()->value.first)  // End of edge is contour
                                                                                               // start (closed contour)
    ) {
        // Move to next edge
        this->_output[contourIndex].edges.insertLast(start->value);
        start = start->next;
    }

    // Add edge ending at intersection or start to output
    this->_output[contourIndex].edges.insertLast(start->value);

    return start->value.second;
}

/**
 * @brief Mark contour with given edge as visited
 *
 * @param edge Edge of polygon
 */
void PolygonOperator::_markContourAsVisited(CircularDLL<Edge>::Node *edge) {
    // Search in first polygon
    for (Contour &contour : this->_first) {
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge == contour.outline.edges.getAt(i)) {
                // Found contour including vertex
                contour.visited = true;
                return;
            }
        }
    }

    // Search in second polygon
    for (Contour &contour : this->_second) {
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge == contour.outline.edges.getAt(i)) {
                // Found contour including vertex
                contour.visited = true;
                return;
            }
        }
    }
}

/**
 * @brief Add point of intersection to list if not already in list
 *
 * @param intersections List of intersections
 * @param intersection Index of vertex at intersection
 */
void PolygonOperator::_addIntersectionIfNeeded(std::list<uint32_t> &intersections, uint32_t intersection) {
    if (std::find(intersections.begin(), intersections.end(), intersection) == intersections.end()) {
        intersections.push_back(intersection);
    }
}

/**
 * @brief Remove vertices from list of intersections if no edges start at intersection
 *
 * @param intersections List of intersections
 * @param contours List of contours containing edges to search
 */
void PolygonOperator::_removeUnwantedIntersections(std::list<uint32_t> &intersections,
                                                   const std::vector<Contour> &contours) {
    std::erase_if(intersections, [&](uint32_t intersection) {
        for (const Contour &contour : contours) {
            for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
                if (contour.outline.edges.getAt(i)->value.first == intersection ||
                    contour.outline.edges.getAt(i)->value.second == intersection) {
                    return false;  // Valid intersection
                }
            }
        }

        return true;  // No edges starting or ending at intersection
    });
}

/**
 * @brief Get edges from first and second polygon that start at given vertex
 *
 * @param vertex Vertex
 *
 * @return Edges starting at vertex
 */
std::vector<CircularDLL<Edge>::Node *> PolygonOperator::_getEdgesStartingAt(uint32_t vertex) {
    std::vector<CircularDLL<Edge>::Node *> edges;

    // Search in first polygon
    for (Contour &contour : this->_first) {
        CircularDLL<Edge>::Node *edge = contour.outline.edges.getFirst();
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge->value.first == vertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }
    }

    // Search in second polygon
    for (Contour &contour : this->_second) {
        CircularDLL<Edge>::Node *edge = contour.outline.edges.getFirst();
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge->value.first == vertex) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }
    }

    return edges;
}

/**
 * @brief Get the contour containing specified edge
 *
 * @param edge Specified edge
 *
 * @return Contout containing edge
 */
const Contour &PolygonOperator::_getContourOfEdge(CircularDLL<Edge>::Node *edge) {
    // Search in first polygon
    for (const Contour &contour : this->_first) {
        CircularDLL<Edge>::Node *contourEdge = contour.outline.edges.getFirst();
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge == contourEdge) {
                return contour;
            }

            contourEdge = contourEdge->next;
        }
    }

    // Search in second polygon
    for (const Contour &contour : this->_second) {
        CircularDLL<Edge>::Node *contourEdge = contour.outline.edges.getFirst();
        for (unsigned int i = 0; i < contour.outline.edges.size(); i++) {
            if (edge == contourEdge) {
                return contour;
            }

            contourEdge = contourEdge->next;
        }
    }

    throw std::runtime_error("PolygonOperator::_getContourOfEdge(): Edge does not belong to any polygon");
}

/**
 * @brief Get orientation for a part of a contour. The part starts and ends in the same vertex.
 *
 * @param outline Full contour
 * @param start Starting edge of subcontour
 *
 * @return Orienatation of subcontour
 */
const Outline::Orientation &PolygonOperator::_getOrientationOfSubcontour(const Outline &outline,
                                                                         CircularDLL<Edge>::Node *start) {
    CircularDLL<Edge>::Node *current = start;
    double area = 0;
    bool isPartOfSubcontour = true;

    do {
        if (isPartOfSubcontour) {
            area += this->_vertices[current->value.first].x * this->_vertices[current->value.second].y -
                    this->_vertices[current->value.second].x * this->_vertices[current->value.first].y;
        }

        // Move to next edge
        current = current->next;

        // Get all edges starting at current vertex
        std::vector<CircularDLL<Edge>::Node *> edges;
        CircularDLL<Edge>::Node *edge = outline.edges.getFirst();
        for (unsigned int i = 0; i < outline.edges.size(); i++) {
            if (edge->value.first == current->value.first) {
                edges.push_back(edge);
            }

            edge = edge->next;
        }

        // Skip edges that are not part of subcontour
        if (edges.size() >= 2) {
            isPartOfSubcontour = !isPartOfSubcontour;
        }
    } while (current != start);

    return area >= 0 ? Outline::Orientation::CCW : Outline::Orientation::CW;
}

double PolygonOperator::_signedAreaOfContour(const Outline &outline) {
    CircularDLL<Edge>::Node *current = outline.edges.getFirst();
    double area = 0;

    for (unsigned int i = 0; i < outline.edges.size(); i++) {
        area += this->_vertices[current->value.first].x * this->_vertices[current->value.second].y -
                this->_vertices[current->value.second].x * this->_vertices[current->value.first].y;
        current = current->next;
    }

    return area;
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
bool PolygonOperator::_isOnLeftSide(glm::vec2 lineStartingPoint, glm::vec2 lineEndingPoint, glm::vec2 point) {
    float a = lineEndingPoint.y - lineStartingPoint.y;
    float b = lineStartingPoint.x - lineEndingPoint.x;
    float c = lineEndingPoint.x * lineStartingPoint.y - lineStartingPoint.x * lineEndingPoint.y;
    float d = a * point.x + b * point.y + c;

    return d < 0;
}

/**
 * @brief Calculates the determinant of a 2x2 matrix
 *
 * @param a Top left value
 * @param b Top right value
 * @param c Bottom left value
 * @param d Bottom right value
 *
 * @return Determinant
 */
double PolygonOperator::_determinant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

/**
 * @brief Checks whether the second edge fully lies on first edge
 *
 * @param first First edge
 * @param second Second edge
 *
 * @return True if second edge fully lies on second edge
 */
bool PolygonOperator::_isEdgeOnEdge(Edge first, Edge second) {
    return this->_isPointOnEdge(this->_vertices.at(first.first), second) &&
           this->_isPointOnEdge(this->_vertices.at(first.second), second);
}

/**
 * @brief Check whether a point lies on edge
 *
 * @param point Point oto check
 * @param edge Given edge
 *
 * @return True if point lies on edge
 */
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

/**
 * @brief Set the maximum error
 *
 * @param epsilon Maximum error
 */
void PolygonOperator::setEpsilon(double epsilon) {
    if (epsilon < 0) {
        throw std::invalid_argument("PolygonOperator::setEpsilon(): Epsilon must be positive");
    }

    this->_epsilon = epsilon;
}

/**
 * @brief Get the final polygon after performing a boolean operation
 *
 * @return Output polygon
 */
std::vector<Outline> PolygonOperator::getPolygon() {
    return this->_output;
}

/**
 * @brief Get vertices of final polygon (still contains all vertices before performing boolean operation, but also
 * contains new intersections)
 *
 * @return Vertices of final polygon
 */
std::vector<glm::vec2> PolygonOperator::getVertices() {
    return this->_vertices;
}

}  // namespace vft
