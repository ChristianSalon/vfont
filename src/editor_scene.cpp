/**
 * @file editor_scene.cpp
 * @author Christian Saloň
 */

#include "editor_scene.h"

const std::string EditorScene::ARIAL_PATH = "Arial-Regular.ttf";

EditorScene::EditorScene(CameraType cameraType) : Scene{cameraType} {
    this->_arial32 = std::make_shared<vft::Font>(EditorScene::ARIAL_PATH, 32);
    this->_textBlock = std::make_shared<vft::TextBlock>(this->_arial32, glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1, true);
    vft::TextRenderer::getInstance().add(this->_textBlock);

    this->_window->setKeypressCallback([&](uint32_t codePoint) {
        this->updateText(codePoint);
    });
}

EditorScene::~EditorScene() {}

void EditorScene::updateText(uint32_t codePoint) {
    if(codePoint == vft::TextRenderer::U_BACKSPACE) {
        this->_textBlock->remove();
    }
    else {
        this->_textBlock->add({codePoint});
    }
}
