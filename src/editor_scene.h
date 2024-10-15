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

    static const std::string ROBOTO_PATH;

private:

    std::shared_ptr<vft::Font> _roboto;
    std::shared_ptr<vft::TextBlock> _textBlock;

public:

    EditorScene(CameraType cameraType);
    ~EditorScene();

private:

    void _updateText(uint32_t codePoint);

};
