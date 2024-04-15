/**
 * @file editor_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>
#include <string>

#include "scene.h"
#include "text_renderer.h"
#include "text_renderer_utils.h"
#include "text_block.h"
#include "font.h"

 /**
  * @class EditorScene
  */
class EditorScene : public Scene {

public:

    static const std::string ARIAL_PATH;

private:

    std::shared_ptr<Font> _arial32;
    std::shared_ptr<TextBlock> _textBlock;

public:

    EditorScene(CameraType cameraType);
    ~EditorScene();

    void updateText(uint32_t codePoint);

};
