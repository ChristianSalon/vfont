/**
 * @file editor_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <glm/vec4.hpp>

#include <VFONT/font.h>
#include <VFONT/text_block.h>
#include <VFONT/text_block_builder.h>
#include <VFONT/text_renderer.h>
#include <VFONT/unicode.h>

#include "scene.h"

class EditorScene : public Scene {
private:
    std::shared_ptr<vft::Font> _font;
    std::shared_ptr<vft::TextBlock> _textBlock;

public:
    EditorScene(CameraType cameraType,
                vft::TessellationStrategy tessellationAlgorithm,
                std::string font,
                unsigned int fontSize,
                bool measureTime = false);
    ~EditorScene();

private:
    void _updateText(uint32_t codePoint);
};
