/**
 * @file glyph_compositor.cpp
 * @author Christian Saloň
 */

#include "CDT.h"

#include "glyph_compositor.h"

namespace vft {

/**
 * @brief Performs constrained delaunay triangulation on a glyph
 */
std::vector<uint32_t> GlyphCompositor::triangulate(std::vector<glm::vec2> &vertices, std::vector<vft::Edge> &edges) {
    // CDT uses Constrained Delaunay Triangulation algorithm
    CDT::Triangulation<float> cdt{CDT::VertexInsertionOrder::Auto, CDT::IntersectingConstraintEdges::TryResolve,
                                  1.0e-6};

    cdt.insertVertices(
        vertices.begin(), vertices.end(), [](const glm::vec2 &p) { return p.x; },
        [](const glm::vec2 &p) { return p.y; });
    cdt.insertEdges(
        edges.begin(), edges.end(), [](const vft::Edge &e) { return e.first; },
        [](const vft::Edge &e) { return e.second; });
    cdt.eraseOuterTrianglesAndHoles();

    // Create index buffer from triangulation
    std::vector<uint32_t> glyphIndices;
    CDT::TriangleVec cdtTriangles = cdt.triangles;
    for (int i = 0; i < cdtTriangles.size(); i++) {
        glyphIndices.push_back(cdtTriangles.at(i).vertices.at(0));
        glyphIndices.push_back(cdtTriangles.at(i).vertices.at(1));
        glyphIndices.push_back(cdtTriangles.at(i).vertices.at(2));
    }

    return glyphIndices;
}

}  // namespace vft
