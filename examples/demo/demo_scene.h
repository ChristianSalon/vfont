/**
 * @file demo_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <VFONT/font.h>
#include <VFONT/text_block.h>
#include <VFONT/text_block_builder.h>
#include <VFONT/text_renderer.h>
#include <VFONT/unicode.h>

#include "scene.h"

class DemoScene : public Scene {
public:
    static const std::string JERSEY_PATH;
    static const std::string CRIMSON_TEXT_PATH;
    static const std::string ROBOTO_PATH;
    static const std::string ROBOTO_MONO_PATH;
    static const std::string NOTO_SANS_JP_PATH;
    static const std::string NOTO_EMOJI_PATH;

    static const std::u8string ENGLISH_TEXT;
    static const std::u16string SLOVAK_TEXT;
    static const std::u32string JAPANESE_TEXT;
    static const std::u32string EMOJI_TEXT;

private:
    std::shared_ptr<vft::Font> _jersey;
    std::shared_ptr<vft::Font> _crimsontext;
    std::shared_ptr<vft::Font> _font;
    std::shared_ptr<vft::Font> _robotomono;
    std::shared_ptr<vft::Font> _notosansjp;
    std::shared_ptr<vft::Font> _notoemoji;

    std::shared_ptr<vft::TextBlock> _block1;
    std::shared_ptr<vft::TextBlock> _block2;
    std::shared_ptr<vft::TextBlock> _block3;
    std::shared_ptr<vft::TextBlock> _block4;
    std::shared_ptr<vft::TextBlock> _block5;
    std::shared_ptr<vft::TextBlock> _block6;
    std::shared_ptr<vft::TextBlock> _block7;

public:
    DemoScene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool measureTime = false);
    ~DemoScene();
};
