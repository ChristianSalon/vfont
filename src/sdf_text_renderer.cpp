/**
 * @file sdf_text_renderer.cpp
 * @author Christian Saloň
 */

#include "sdf_text_renderer.h"

namespace vft {

/**
 * @brief Initialize sdf text renderer, uses antialiasing
 *
 * @param softEdgeMin Minimum distance threshold used for aplha blending
 * @param softEdgeMax Maximum distance threshold used for aplha blending
 */
SdfTextRenderer::SdfTextRenderer(float softEdgeMin, float softEdgeMax)
    : _useSoftEdges{true}, _softEdgeMin{softEdgeMin}, _softEdgeMax{softEdgeMax} {
    this->_tessellator = std::make_unique<SdfTessellator>();
}

/**
 * @brief Initialize sdf text renderer, does not use antialiasing
 */
SdfTextRenderer::SdfTextRenderer() {
    this->_tessellator = std::make_unique<SdfTessellator>();
}

/**
 * @brief Recreates vertex and index buffers for all characters in text block
 */
void SdfTextRenderer::update() {
    this->_vertices.clear();
    this->_boundingBoxIndices.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t boundingBoxIndexCount = 0;

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

                // Check if glyph has geometry
                const Glyph &glyph = this->_cache->getGlyph(key);
                if (glyph.mesh.getVertexCount() == 0 ||
                    glyph.mesh.getIndexCount(SdfTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX) == 0) {
                    this->_offsets.insert({key, GlyphInfo{0, 0}});
                    continue;
                }

                // Compute buffer offsets
                this->_offsets.insert(
                    {key, GlyphInfo{boundingBoxIndexCount,
                                    glyph.mesh.getIndexCount(SdfTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX)}});

                // Get uv coordinnates from font atlas
                if (!this->_fontAtlases.contains(character.getFont()->getFontFamily())) {
                    throw std::runtime_error("VulkanSdfTextRenderer::update(): Font atlas for font " +
                                             character.getFont()->getFontFamily() + " was not found");
                }

                FontAtlas::GlyphInfo glyphInfo =
                    this->_fontAtlases.at(character.getFont()->getFontFamily()).getGlyph(character.getGlyphId());
                glm::vec2 uvTopLeft = glyphInfo.uvTopLeft;
                glm::vec2 uvBottomRight = glyphInfo.uvBottomRight;
                glm::vec2 uvTopRight{uvBottomRight.x, uvTopLeft.y};
                glm::vec2 uvBottomLeft{uvTopLeft.x, uvBottomRight.y};

                // Insert bounding box vertices to vertex buffer
                this->_vertices.push_back(Vertex{glyph.mesh.getVertices().at(0), uvBottomLeft});
                this->_vertices.push_back(Vertex{glyph.mesh.getVertices().at(1), uvTopLeft});
                this->_vertices.push_back(Vertex{glyph.mesh.getVertices().at(2), uvTopRight});
                this->_vertices.push_back(Vertex{glyph.mesh.getVertices().at(3), uvBottomRight});

                // Insert bounding box indices to index buffer
                this->_boundingBoxIndices.insert(
                    this->_boundingBoxIndices.end(),
                    glyph.mesh.getIndices(SdfTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(SdfTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).end());

                // Add an offset to bounding box indices of current character
                for (unsigned int j = boundingBoxIndexCount; j < this->_boundingBoxIndices.size(); j++) {
                    this->_boundingBoxIndices[j] += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                boundingBoxIndexCount += glyph.mesh.getIndexCount(SdfTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX);
            }
        }
    }
}

/**
 * @brief Add font atlas used for rendering glyphs
 *
 * @param atlas Font atlas to add
 */
void SdfTextRenderer::addFontAtlas(const FontAtlas &atlas) {
    this->_fontAtlases.insert({atlas.getFontFamily(), atlas});
}

}  // namespace vft
