/**
 * @file editor_scene.cpp
 * @author Christian Saloň
 */

#include "editor_scene.h"

#include <glm/vec4.hpp>

const std::string EditorScene::BITSTREAM_VERA_PATH = "Vera.ttf";

EditorScene::EditorScene(CameraType cameraType) : Scene{cameraType} {
    this->_bitstreamvera = std::make_shared<vft::Font>(EditorScene::BITSTREAM_VERA_PATH);
    this->_textBlock = std::make_shared<vft::TextBlock>(this->_bitstreamvera, 32, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1);
    this->renderer.add(this->_textBlock);

    this->_window->setKeypressCallback([&](uint32_t codePoint) {
        this->updateText(codePoint);
    });
}

EditorScene::~EditorScene() {}

void EditorScene::updateText(uint32_t codePoint) {
    if(codePoint == vft::U_BACKSPACE) {
        this->_textBlock->remove();
    }
    else {
        this->_textBlock->add({codePoint});
    }
}
