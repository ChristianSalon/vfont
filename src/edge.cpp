/**
 * @file edge.cpp
 * @author Christian Saloň
 */

#include "edge.h"

namespace vft {

/**
 * @brief Edge constructor
 *
 * @param first Index of edge starting vertex
 * @param second Index of edge ending vertex
 */
Edge::Edge(uint32_t first, uint32_t second) : first{first}, second{second} {}

/**
 * @brief Edge constructor, sets indices to zero
 */
Edge::Edge() : first{0}, second{0} {}

/**
 * @brief Edge equality operator
 *
 * @param edge Edge to compare
 *
 * @return True if first index and second index of edges match
 */
bool Edge::operator==(const Edge &edge) const {
    return this->first == edge.first && this->second == edge.second;
}

/**
 * @brief Checks if edges are inverse
 *
 * @param edge Edge to compare
 *
 * @return True if the first index from first edge matches second index of second edge and the second index from first
 * edge matches first index of second edge
 */
bool Edge::isInverse(const Edge &edge) const {
    return this->first == edge.second && this->second == edge.first;
}

}  // namespace vft
