/**
 * @file demo_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>
#include <string>

#include "scene.h"
#include "base_camera.h"
#include "text_renderer.h"
#include "text_renderer_utils.h"
#include "text_block.h"
#include "font.h"

 /**
  * @class DemoScene
  */
class DemoScene : public Scene {

public:

    static const std::string ARIAL_PATH;
    static const std::string JERSEY_PATH;
    static const std::string CRIMSON_TEXT_PATH;
    static const std::string ROBOTO_MONO_PATH;
    static const std::string NOTO_SANS_JP_PATH;

    static const std::vector<uint32_t> ENGLISH_CODE_POINTS;
    static const std::vector<uint32_t> SLOVAK_CODE_POINTS;
    static const std::vector<uint32_t> JAPANESE_CODE_POINTS;

private:

    std::shared_ptr<Font> _arial32;
    std::shared_ptr<Font> _arial64;
    std::shared_ptr<Font> _jersey32;
    std::shared_ptr<Font> _crimsontext32;
    std::shared_ptr<Font> _robotomono32;
    std::shared_ptr<Font> _notosansjp32;

    std::shared_ptr<TextBlock> _block1;
    std::shared_ptr<TextBlock> _block2;
    std::shared_ptr<TextBlock> _block3;
    std::shared_ptr<TextBlock> _block4;
    std::shared_ptr<TextBlock> _block5;
    std::shared_ptr<TextBlock> _block6;

public:

    DemoScene(CameraType cameraType);
    ~DemoScene();

};
