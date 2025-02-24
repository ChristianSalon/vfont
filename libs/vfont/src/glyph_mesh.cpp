/**
 * @file glyph_mesh.cpp
 * @author Christian Saloň
 */

#include "glyph_mesh.h"

namespace vft {

GlyphMesh::GlyphMesh(std::vector<glm::vec2> vertices, std::vector<std::vector<uint32_t>> indices)
    : _vertices{vertices}, _indices{indices} {}

GlyphMesh::GlyphMesh() {
    // Resize indices array to prevent index out of range exception in getIndices()
    this->_indices.resize(10);
}

void GlyphMesh::addVertex(glm::vec2 vertex) {
    this->_vertices.push_back(vertex);
}

void GlyphMesh::setVertices(std::vector<glm::vec2> vertices) {
    this->_vertices = vertices;
}

void GlyphMesh::setIndices(unsigned int drawIndex, std::vector<uint32_t> indices) {
    this->_indices.at(drawIndex) = indices;
}

const std::vector<glm::vec2> &GlyphMesh::getVertices() const {
    return this->_vertices;
}

const std::vector<uint32_t> &GlyphMesh::getIndices(unsigned int drawIndex) const {
    if (drawIndex >= this->_indices.size()) {
        throw std::out_of_range("GlyphMesh::getIndices(): Index is out of range");
    }

    return this->_indices.at(drawIndex);
}

uint32_t GlyphMesh::getVertexCount() const {
    return this->_vertices.size();
}

int GlyphMesh::getIndexCount(unsigned int drawIndex) const {
    return this->_indices.at(drawIndex).size();
}

unsigned int GlyphMesh::getDrawCount(unsigned int drawIndex) const {
    return this->_indices.size();
}

}  // namespace vft
