/**
 * @file gpu_tessellator.cpp
 * @author Christian Saloň
 */

#include "gpu_tessellator.h"

namespace vft {

GpuTessellator::GpuTessellator(GlyphCache& cache) : Tessellator{ cache } {}

Glyph GpuTessellator::composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) {
    GlyphKey key{ font->getFontFamily(), codePoint };
    if (this->_cache.exists(key)) {
        return this->_cache.getGlyph(key);
    }

    Glyph glyph = GpuTessellator::_composeGlyph(codePoint, font);

    std::vector<glm::vec2> vertices = glyph.mesh.getVertices();
    std::vector<vft::Edge> lineSegments = glyph.getLineSegmentsIndices();
    std::vector<vft::Curve> curveSegments = glyph.getCurveSegmentsIndices();

    // Create bounding box indices that form two triangles
    uint32_t newVertexIndex = vertices.size();
    std::vector<uint32_t> boundingBoxIndices = { newVertexIndex, newVertexIndex + 3, newVertexIndex + 1, newVertexIndex + 2, newVertexIndex + 1, newVertexIndex + 3 };

    // Add bounding box vertices to vertex buffer if neccessary
    std::array<glm::vec2, 4> boundingBoxVertices = glyph.getBoundingBox();
    for (glm::vec2 vertex : boundingBoxVertices) {
        vertices.push_back(vertex);
        newVertexIndex++;
    }

    // Make line segments continuous
    for (vft::Curve curve : curveSegments) {
        if (GpuTessellator::_isOnRightSide(vertices.at(curve.start), vertices.at(curve.end), vertices.at(curve.control))) {
            lineSegments.push_back({ curve.start, curve.control });
            lineSegments.push_back({ curve.control, curve.end });
        }
        else {
            lineSegments.push_back({ curve.start, curve.end });
        }
    }

    // Create line segments index buffer
    std::vector<uint32_t> lineIndices;
    for (vft::Edge edge : lineSegments) {
        lineIndices.push_back(edge.first);
        lineIndices.push_back(edge.second);
    }

    // Create curve segments index buffer
    std::vector<uint32_t> curveIndices;
    for (vft::Curve curve : curveSegments) {
        curveIndices.push_back(curve.start);
        curveIndices.push_back(curve.control);
        curveIndices.push_back(curve.end);
    }

    GlyphMesh mesh{ vertices, { boundingBoxIndices, curveIndices, lineIndices } };
    glyph.mesh = mesh;
    this->_cache.setGlyph(key, glyph);

    return glyph;
}

}
