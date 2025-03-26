/**
 * @file tessellation_shaders_text_renderer.cpp
 * @author Christian Saloň
 */

#include "tessellation_shaders_text_renderer.h"

namespace vft {

/**
 * @brief Initialize tessellation shaders text renderer
 */
TessellationShadersTextRenderer::TessellationShadersTextRenderer() {
    this->_tessellator = std::make_unique<TessellationShadersTessellator>();
}

/**
 * @brief Recreates vertex and index buffers for all characters in text block
 */
void TessellationShadersTextRenderer::update() {
    this->_vertices.clear();
    this->_lineSegmentsIndices.clear();
    this->_curveSegmentsIndices.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t lineSegmentsIndexCount = 0;
    uint32_t curveSegmentsIndexCount = 0;

    for (unsigned int i = 0; i < this->_textBlocks.size(); i++) {
        for (const Character &character : this->_textBlocks[i]->getCharacters()) {
            GlyphKey key{character.getFont()->getFontFamily(), character.getGlyphId(), 0};

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
                    {key,
                     GlyphInfo{
                         lineSegmentsIndexCount,
                         glyph.mesh.getIndexCount(TessellationShadersTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX),
                         curveSegmentsIndexCount,
                         glyph.mesh.getIndexCount(TessellationShadersTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX)}});

                // Create vertex buffer
                this->_vertices.insert(this->_vertices.end(), glyph.mesh.getVertices().begin(),
                                       glyph.mesh.getVertices().end());

                // Create line segments index buffer
                this->_lineSegmentsIndices.insert(
                    this->_lineSegmentsIndices.end(),
                    glyph.mesh.getIndices(TessellationShadersTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(TessellationShadersTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX).end());

                // Create curve segments index buffer
                this->_curveSegmentsIndices.insert(
                    this->_curveSegmentsIndices.end(),
                    glyph.mesh.getIndices(TessellationShadersTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(TessellationShadersTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX).end());

                // Add an offset to line segment indices of current character
                for (int j = lineSegmentsIndexCount; j < this->_lineSegmentsIndices.size(); j++) {
                    this->_lineSegmentsIndices.at(j) += vertexCount;
                }

                // Add an offset to curve segment indices of current character
                for (int j = curveSegmentsIndexCount; j < this->_curveSegmentsIndices.size(); j++) {
                    this->_curveSegmentsIndices.at(j) += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                lineSegmentsIndexCount +=
                    glyph.mesh.getIndexCount(TessellationShadersTessellator::GLYPH_MESH_TRIANGLE_BUFFER_INDEX);
                curveSegmentsIndexCount +=
                    glyph.mesh.getIndexCount(TessellationShadersTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX);
            }
        }
    }
}

}  // namespace vft
