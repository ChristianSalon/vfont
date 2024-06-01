/**
 * @file editor_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "scene.h"

 /**
  * @class EditorScene
  */
class EditorScene : public Scene {

public:

    static const std::string BITSTREAM_VERA_PATH;

private:

    std::shared_ptr<vft::Font> _bitstreamvera;
    std::shared_ptr<vft::TextBlock> _textBlock;

public:

    EditorScene(CameraType cameraType);
    ~EditorScene();

    void updateText(uint32_t codePoint);

};
