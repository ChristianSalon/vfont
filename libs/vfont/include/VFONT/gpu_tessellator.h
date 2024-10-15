/**
 * @file gpu_tessellator.h
 * @author Christian Saloň
 */

#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cstdint>

#include "tessellator.h"
#include "font.h"
#include "glyph.h"
#include "glyph_compositor.h"
#include "glyph_mesh.h"
#include "glyph_cache.h"
#include "text_renderer_utils.h"

namespace vft {

class GpuTessellator : public Tessellator {

public:

    GpuTessellator(GlyphCache& cache);
    ~GpuTessellator() = default;

    Glyph composeGlyph(uint32_t codePoint, std::shared_ptr<vft::Font> font) override;

};

}
