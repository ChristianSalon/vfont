/**
 * @file glyph.cpp
 * @author Christian Saloň
 */

#include <vector>

#include "glyph.h"

namespace vft {

/**
 * @brief Glyph constructor
 */
Glyph::Glyph() {
    this->_advanceX = 0;
    this->_advanceY = 0;
}

/**
 * @brief Add new vertex to vertex buffer
 * 
 * @param vertex New vertex
 */
void Glyph::addVertex(glm::vec2 vertex) {
    this->_vertices.push_back(vertex);
}

/**
 * @brief Update vertex in vertex buffer at given index
 * 
 * @param index Index of vertex in vertex buffer which value is being changed
 * @param vertex New value of vertex
 */
void Glyph::updateVertex(int index, glm::vec2 vertex) {
    this->_vertices.at(index) = vertex;
}

/**
 * @brief Add new edge of glyph
 * 
 * @param edge New edge
 */
void Glyph::addEdge(vft::Edge edge) {
    if(edge.first != edge.second) {
        this->_edges.push_back(edge);
    }
}

/**
 * @brief Update edge at given index
 *
 * @param index Index of edge which is being changed
 * @param vertex New edge
 */
void Glyph::updateEdge(int index, vft::Edge edge) {
    if(edge.first != edge.second) {
        this->_edges.at(index) = edge;
    }
}

/**
 * @brief Add index to index buffer
 * 
 * @param index New index of vertex
 */
void Glyph::addIndex(uint32_t index) {
    this->_indices.push_back(index);
}

/**
 * @brief Set by how much should the X coordinate update after rendering glyph
 * 
 * @param advanceX X value of advance vector
 */
void Glyph::setAdvanceX(int advanceX) {
    this->_advanceX = advanceX;
}

/**
 * @brief Set by how much should the Y coordinate update after rendering glyph
 *
 * @param advanceX Y value of advance vector
 */
void Glyph::setAdvanceY(int advanceY) {
    this->_advanceY = advanceY;
}

/**
 * @brief Set vertex buffer
 * 
 * @param vertices New vertex buffer
 */
void Glyph::setVertices(const std::vector<glm::vec2> &vertices) {
    this->_vertices = vertices;
}

/**
 * @brief Set index buffer
 *
 * @param indices New index buffer
 */
void Glyph::setIndices(const std::vector<uint32_t> &indices) {
    this->_indices = indices;
}

/**
 * @brief Set glypg edges
 *
 * @param edges New edges of glyph
 */
void Glyph::setEdges(const std::vector<vft::Edge> &edges) {
    this->_edges = edges;
}

/**
 * @brief Get by how much should the X coordinate update after rendering glyph
 * 
 * @return X value of advance vector
 */
int Glyph::getAdvanceX() const {
    return this->_advanceX;
}

/**
 * @brief Get by how much should the Y coordinate update after rendering glyph
 *
 * @return Y value of advance vector
 */
int Glyph::getAdvanceY() const {
    return this->_advanceY;
}

/**
 * @brief Get vertex buffer
 * 
 * @return Vertex buffer
 */
const std::vector<glm::vec2> &Glyph::getVertices() const {
    return this->_vertices;
}

/**
 * @brief Get index buffer
 *
 * @return Index buffer
 */
const std::vector<uint32_t> &Glyph::getIndices() const {
    return this->_indices;
}

/**
 * @brief Get edges of glyph
 *
 * @return Edges of glyph
 */
const std::vector<vft::Edge> &Glyph::getEdges() const {
    return this->_edges;
}

/**
 * @brief Get the amount of vertices in vertex buffer
 * 
 * @return Amount of vertices in vertex buffer
 */
int Glyph::getVertexCount() const {
    return this->_vertices.size();
}

/**
 * @brief Get the amount of indices in index buffer
 *
 * @return Amount of indices in index buffer
 */
int Glyph::getIndexCount() const {
    return this->_indices.size();
}

/**
 * @brief Get the amount of edges of glyph
 *
 * @return Amount of edges of glyph
 */
int Glyph::getEdgeCount() const {
    return this->_edges.size();
}

}
