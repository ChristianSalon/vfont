/**
 * @file triangulation_text_renderer.cpp
 * @author Christian Saloň
 */

#include "triangulation_text_renderer.h"

namespace vft {

/**
 * @brief Initialize triangulation text renderer
 */
TriangulationTextRenderer::TriangulationTextRenderer() {
    this->_tessellator = std::make_unique<TriangulationTessellator>();
}

/**
 * @brief Recreates vertex and index buffers for all characters in text block
 */
void TriangulationTextRenderer::update() {
    this->_vertices.clear();
    this->_indices.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    for (unsigned int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), character.getFontSize()};

            if (!this->_offsets.contains(key)) {
                // Insert glyph into cache
                if (!this->_cache->exists(key)) {
                    this->_cache->setGlyph(key,
                                           this->_tessellator->composeGlyph(character.getGlyphId(), character.getFont(),
                                                                            character.getFontSize()));
                }

                // Compute buffer offsets
                const Glyph &glyph = this->_cache->getGlyph(key);
                this->_offsets.insert(
                    {key, GlyphInfo{indexCount, glyph.mesh.getIndexCount(
                                                    TriangulationTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX)}});

                // Insert glyph mesh into vertex and index buffer
                this->_vertices.insert(this->_vertices.end(), glyph.mesh.getVertices().begin(),
                                       glyph.mesh.getVertices().end());
                this->_indices.insert(
                    this->_indices.end(),
                    glyph.mesh.getIndices(TriangulationTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(TriangulationTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX).end());

                // Add an offset to line segment indices of current character
                for (int j = indexCount; j < this->_indices.size(); j++) {
                    this->_indices.at(j) += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                indexCount += glyph.mesh.getIndexCount(TriangulationTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX);
            }
        }
    }
}

}  // namespace vft
