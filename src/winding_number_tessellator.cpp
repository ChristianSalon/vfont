/**
 * @file winding_number_tessellator.cpp
 * @author Christian Saloň
 */

#include "winding_number_tessellator.h"

namespace vft {

/**
 * @brief WindingNumberTessellator constructor
 */
WindingNumberTessellator::WindingNumberTessellator() {}

/**
 * @brief Composes a glyph ready for rendering
 *
 * @param glyphId Id of glyph to compose
 * @param font Font of glyph
 * @param fontSize Font size of glyph
 */
Glyph WindingNumberTessellator::composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize) {
    GlyphKey key{font->getFontFamily(), glyphId, 0};
    Glyph glyph = this->_composeGlyph(glyphId, font);

    std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
    std::vector<Edge> lineSegments = glyph.getLineSegmentsIndices();
    std::vector<Curve> curveSegments = glyph.getCurveSegmentsIndices();

    // Create bounding box indices that form two triangles
    uint32_t newVertexIndex = vertices.size();
    std::vector<uint32_t> boundingBoxIndices = {newVertexIndex,     newVertexIndex + 3, newVertexIndex + 1,
                                                newVertexIndex + 2, newVertexIndex + 1, newVertexIndex + 3};

    // Add bounding box vertices to vertex buffer if neccessary
    std::array<glm::vec2, 4> boundingBoxVertices = glyph.getBoundingBox();
    for (glm::vec2 vertex : boundingBoxVertices) {
        vertices.push_back(vertex);
        newVertexIndex++;
    }

    // Create line segments index buffer
    std::vector<uint32_t> lineIndices;
    for (Edge edge : lineSegments) {
        lineIndices.push_back(edge.first);
        lineIndices.push_back(edge.second);
    }

    // Create curve segments index buffer
    std::vector<uint32_t> curveIndices;
    for (Curve curve : curveSegments) {
        curveIndices.push_back(curve.start);
        curveIndices.push_back(curve.control);
        curveIndices.push_back(curve.end);
    }

    GlyphMesh mesh{vertices, {boundingBoxIndices, curveIndices, lineIndices}};
    glyph.mesh = mesh;

    return glyph;
}

}  // namespace vft
