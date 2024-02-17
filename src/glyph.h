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
 * @brief Represents a character to render
 */
class Glyph {

private:

    std::vector<tr::vertex_t> _vertices;    /**< Glyph vertices */
    std::vector<uint32_t> _indices;         /**< Glyph vertex indices */
    std::vector<tr::edge_t> _edges;         /**< Glyph edges */

    int _advanceX;
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
