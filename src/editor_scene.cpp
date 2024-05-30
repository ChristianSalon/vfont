/**
 * @file editor_scene.cpp
 * @author Christian Saloň
 */

#include "editor_scene.h"

#include <glm/vec4.hpp>

const std::string EditorScene::ARIAL_PATH = "Arial-Regular.ttf";

EditorScene::EditorScene(CameraType cameraType) : Scene{cameraType} {
    this->_arial = std::make_shared<vft::Font>(EditorScene::ARIAL_PATH);
    this->_textBlock = std::make_shared<vft::TextBlock>(this->_arial, 32, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1);
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
