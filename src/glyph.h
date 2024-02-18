/**
 * @file glyph.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>

#include "text_renderer_utils.h"

/**
 * @class Glyph
 *
 * @brief Represents glyph vertex data and metrics
 */
class Glyph {

private:

    std::vector<tr::vertex_t> _vertices;    /**< Glyph vertices */
    std::vector<uint32_t> _indices;         /**< Glyph vertex indices */
    std::vector<tr::edge_t> _edges;         /**< Glyph edges */

    /**
     * Indicates by how much to increment the X coordinate of pen position.
     * This includes the glyph's width plus the space behind.
     * Used for horizontal layouts
     */
    int _advanceX;

    /**
     * Indicates by how much to increment the Y coordinate of pen position.
     * Not specified for horizontal layouts
     */
    int _advanceY;

public:

    Glyph();

    void addVertex(tr::vertex_t vertex);
    void updateVertex(int index, tr::vertex_t vertex);

    void addEdge(tr::edge_t edge);
    void updateEdge(int index, tr::edge_t edge);

    void addIndex(uint32_t index);

    void setAdvanceX(int advanceX);
    void setAdvanceY(int advanceY);

    int getAdvanceX();
    int getAdvanceY();

    std::vector<tr::vertex_t> &getVertices();
    std::vector<uint32_t> &getIndices();
    std::vector<tr::edge_t> &getEdges();

    int getVertexCount();
    int getIndexCount();
    int getEdgeCount();

};
