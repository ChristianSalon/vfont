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

void Glyph::addVertex(glm::vec2 vertex) {
    this->_vertices.push_back(vertex);
}

void Glyph::updateVertex(int index, glm::vec2 vertex) {
    this->_vertices.at(index) = vertex;
}

void Glyph::addEdge(vft::edge_t edge) {
    this->_edges.push_back(edge);
}

void Glyph::updateEdge(int index, vft::edge_t edge) {
    this->_edges.at(index) = edge;
}

void Glyph::addIndex(uint32_t index) {
    this->_indices.push_back(index);
}

void Glyph::setAdvanceX(int advanceX) {
    this->_advanceX = advanceX;
}

void Glyph::setAdvanceY(int advanceY) {
    this->_advanceY = advanceY;
}

void Glyph::setVertices(const std::vector<glm::vec2> &vertices) {
    this->_vertices = vertices;
}

void Glyph::setIndices(const std::vector<uint32_t> &indices) {
    this->_indices = indices;
}

void Glyph::setEdges(const std::vector<vft::edge_t> &edges) {
    this->_edges = edges;
}

int Glyph::getAdvanceX() const {
    return this->_advanceX;
}

int Glyph::getAdvanceY() const {
    return this->_advanceY;
}

const std::vector<glm::vec2> &Glyph::getVertices() const {
    return this->_vertices;
}

const std::vector<uint32_t> &Glyph::getIndices() const {
    return this->_indices;
}

const std::vector<vft::edge_t> &Glyph::getEdges() const {
    return this->_edges;
}

int Glyph::getVertexCount() const {
    return this->_vertices.size();
}

int Glyph::getIndexCount() const {
    return this->_indices.size();
}

int Glyph::getEdgeCount() const {
    return this->_edges.size();
}

}
