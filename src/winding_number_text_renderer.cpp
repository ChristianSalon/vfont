/**
 * @file winding_number_text_renderer.cpp
 * @author Christian Saloň
 */

#include "winding_number_text_renderer.h"

namespace vft {

/**
 * @brief Initialize winding number text renderer
 */
WindingNumberTextRenderer::WindingNumberTextRenderer() {
    this->_tessellator = std::make_unique<WindingNumberTessellator>();
}

/**
 * @brief Recreates vertex, index and segment buffers for all characters in text block
 */
void WindingNumberTextRenderer::update() {
    this->_vertices.clear();
    this->_boundingBoxIndices.clear();
    this->_segments.clear();
    this->_segmentsInfo.clear();
    this->_offsets.clear();

    uint32_t vertexCount = 0;
    uint32_t boundingBoxIndexCount = 0;
    uint32_t segmentsCount = 0;
    uint32_t segmentsInfoCount = 0;

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
                     GlyphInfo{boundingBoxIndexCount,
                               glyph.mesh.getIndexCount(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX),
                               segmentsInfoCount}});

                // Create vertex buffer
                this->_vertices.insert(this->_vertices.end(), glyph.mesh.getVertices().begin(),
                                       glyph.mesh.getVertices().end());
                this->_boundingBoxIndices.insert(
                    this->_boundingBoxIndices.end(),
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).begin(),
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX).end());

                // Create line and curve segments buffer
                std::vector<glm::vec2> vertices = glyph.mesh.getVertices();

                std::vector<uint32_t> lineSegments =
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_LINE_BUFFER_INDEX);
                uint32_t lineCount = lineSegments.size() / 2;
                for (int j = 0; j < lineSegments.size(); j += 2) {
                    this->_segments.push_back(vertices.at(lineSegments.at(j)));
                    this->_segments.push_back(vertices.at(lineSegments.at(j + 1)));
                }

                std::vector<uint32_t> curveSegments =
                    glyph.mesh.getIndices(WindingNumberTessellator::GLYPH_MESH_CURVE_BUFFER_INDEX);
                uint32_t curveCount = curveSegments.size() / 3;
                for (int j = 0; j < curveSegments.size(); j += 3) {
                    this->_segments.push_back(vertices.at(curveSegments.at(j)));
                    this->_segments.push_back(vertices.at(curveSegments.at(j + 1)));
                    this->_segments.push_back(vertices.at(curveSegments.at(j + 2)));
                }

                this->_segmentsInfo.push_back(SegmentsInfo{
                    segmentsCount, lineCount, segmentsCount + static_cast<uint32_t>(lineSegments.size()), curveCount});

                // Add an offset to bounding box indices of current character
                for (int j = boundingBoxIndexCount; j < this->_boundingBoxIndices.size(); j++) {
                    this->_boundingBoxIndices.at(j) += vertexCount;
                }

                vertexCount += glyph.mesh.getVertexCount();
                boundingBoxIndexCount +=
                    glyph.mesh.getIndexCount(WindingNumberTessellator::GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX);
                segmentsCount += lineSegments.size() + curveSegments.size();
                segmentsInfoCount++;
            }
        }
    }
}

}  // namespace vft
