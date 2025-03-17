/**
 * @file editor_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <glm/vec4.hpp>

#include <VFONT/font.h>
#include <VFONT/text_block.h>
#include <VFONT/text_renderer.h>
#include <VFONT/text_block_builder.h>

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

    EditorScene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool measureTime = false);
    ~EditorScene();

private:

    void _updateText(uint32_t codePoint);

};
