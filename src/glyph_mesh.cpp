/**
 * @file glyph_mesh.cpp
 * @author Christian Saloň
 */

#include "glyph_mesh.h"

namespace vft {

/**
 * @brief GlyphMesh constructor
 *
 * @param vertices Glyph's vertex buffer
 * @param indices Glyph's index buffers
 */
GlyphMesh::GlyphMesh(std::vector<glm::vec2> vertices, std::vector<std::vector<uint32_t>> indices)
    : _vertices{vertices}, _indices{indices} {}

/**
 * @brief GlyphMesh constructor
 */
GlyphMesh::GlyphMesh() {
    // Resize indices array to prevent index out of range exception in getIndices()
    this->_indices.resize(10);
}

/**
 * @brief Add vertex to vertex buffer
 *
 * @param vertex New Vertex
 */
void GlyphMesh::addVertex(glm::vec2 vertex) {
    this->_vertices.push_back(vertex);
}

/**
 * @brief Set vertex buffer of glyph
 *
 * @param vertices New vertex buffer
 */
void GlyphMesh::setVertices(std::vector<glm::vec2> vertices) {
    this->_vertices = vertices;
}

/**
 * @brief Set one index buffer of glyph
 * @param drawIndex Index pointing to which index buffer to set from vector of index buffers
 * @param indices New index buffer
 */
void GlyphMesh::setIndices(unsigned int drawIndex, std::vector<uint32_t> indices) {
    this->_indices.at(drawIndex) = indices;
}

/**
 * @brief Get vertex buffer of glyph
 *
 * @return Vertex buffer
 */
const std::vector<glm::vec2> &GlyphMesh::getVertices() const {
    return this->_vertices;
}

/**
 * @brief Get index buffer at given index
 *
 * @param drawIndex Index pointing to which index buffer to get from vector of index buffers
 *
 * @return Index buffer
 */
const std::vector<uint32_t> &GlyphMesh::getIndices(unsigned int drawIndex) const {
    if (drawIndex >= this->_indices.size()) {
        throw std::out_of_range("GlyphMesh::getIndices(): Index is out of range");
    }

    return this->_indices.at(drawIndex);
}

/**
 * @brief Get number of vertices in vertex buffer
 *
 * @return Vertex count
 */
uint32_t GlyphMesh::getVertexCount() const {
    return this->_vertices.size();
}

/**
 * @brief Get number if indices in one index buffer
 *
 * @param drawIndex Index pointing to one specific index buffer
 * @return
 */
uint32_t GlyphMesh::getIndexCount(unsigned int drawIndex) const {
    return this->_indices.at(drawIndex).size();
}

/**
 * @brief Get number of index buffers used in mesh
 *
 * @return Index buffer count
 */
unsigned int GlyphMesh::getDrawCount() const {
    return this->_indices.size();
}

}  // namespace vft
