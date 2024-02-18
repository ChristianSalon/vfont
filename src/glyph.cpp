/**
 * @file glyph.cpp
 * @author Christian Saloň
 */

#include <vector>

#include "glyph.h"

/**
 * @brief Glyph constructor
 */
Glyph::Glyph() {
    this->_advanceX = 0;
    this->_advanceY = 0;
}

void Glyph::addVertex(tr::vertex_t vertex) {
    this->_vertices.push_back(vertex);
}

void Glyph::updateVertex(int index, tr::vertex_t vertex) {
    this->_vertices.at(index) = vertex;
}

void Glyph::addEdge(tr::edge_t edge) {
    this->_edges.push_back(edge);
}

void Glyph::updateEdge(int index, tr::edge_t edge) {
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

int Glyph::getAdvanceX() {
    return this->_advanceX;
}

int Glyph::getAdvanceY() {
    return this->_advanceY;
}

std::vector<tr::vertex_t> &Glyph::getVertices() {
    return this->_vertices;
}

std::vector<uint32_t> &Glyph::getIndices() {
    return this->_indices;
}

std::vector<tr::edge_t> &Glyph::getEdges() {
    return this->_edges;
}

int Glyph::getVertexCount() {
    return this->_vertices.size();
}

int Glyph::getIndexCount() {
    return this->_indices.size();
}

int Glyph::getEdgeCount() {
    return this->_edges.size();
}
