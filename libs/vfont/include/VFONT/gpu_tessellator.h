/**
 * @file gpu_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "font.h"
#include "glyph.h"
#include "glyph_cache.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "tessellator.h"
#include "text_renderer_utils.h"

namespace vft {

class GpuTessellator : public Tessellator {
public:
    static constexpr unsigned int GLYPH_MESH_BOUNDING_BOX_BUFFER_INDEX = 0;
    static constexpr unsigned int GLYPH_MESH_CURVE_BUFFER_INDEX = 1;
    static constexpr unsigned int GLYPH_MESH_LINE_BUFFER_INDEX = 2;

    GpuTessellator();
    ~GpuTessellator() = default;

    Glyph composeGlyph(uint32_t glyphId, std::shared_ptr<vft::Font> font, unsigned int fontSize = 0) override;
};

}  // namespace vft
