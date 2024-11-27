/**
 * @file editor_scene.cpp
 * @author Christian Saloň
 */

#include "editor_scene.h"

#include <glm/vec4.hpp>

const std::string EditorScene::ROBOTO_PATH = "assets/Roboto-Regular.ttf";

EditorScene::EditorScene(CameraType cameraType, vft::Renderer::TessellationStrategy tessellationAlgorithm, bool measureTime) : Scene{cameraType, tessellationAlgorithm, measureTime} {
    this->_roboto = std::make_shared<vft::Font>(EditorScene::ROBOTO_PATH);
    this->_textBlock = std::make_shared<vft::TextBlock>(this->_roboto, 32, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1);
    this->_renderer->add(this->_textBlock);

    this->_window->setKeypressCallback([&](uint32_t codePoint) {
        this->_updateText(codePoint);
    });
}

EditorScene::~EditorScene() {}

void EditorScene::_updateText(uint32_t codePoint) {
    if(codePoint == vft::U_BACKSPACE) {
        this->_textBlock->remove();
    }
    else {
        this->_textBlock->add(std::vector<uint32_t>{codePoint});
    }
}
