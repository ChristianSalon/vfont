/**
 * @file tessellation_shaders_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "circular_dll.h"
#include "font.h"
#include "glyph.h"
#include "glyph_cache.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "polygon_operator.h"
#include "tessellator.h"
#include "text_renderer_utils.h"

namespace vft {

class TessellationShadersTessellator : public Tessellator {
public:
    static constexpr unsigned int GLYPH_MESH_TRIANGLE_BUFFER_INDEX = 0;
    static constexpr unsigned int GLYPH_MESH_CURVE_BUFFER_INDEX = 1;

protected:
    std::vector<glm::vec2> _vertices{};
    std::vector<CircularDLL<Edge>> _firstPolygon{};
    std::vector<CircularDLL<Edge>> _secondPolygon{};

public:
    TessellationShadersTessellator();
    ~TessellationShadersTessellator() = default;

    Glyph composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize = 0) override;

protected:
    bool _isOnLeftSide(const glm::vec2 &lineStartingPoint, const glm::vec2 &lineEndingPoint, const glm::vec2 &point);
};

}  // namespace vft
