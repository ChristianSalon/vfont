/**
 * @file editor_scene.cpp
 * @author Christian Saloň
 */

#include "editor_scene.h"

EditorScene::EditorScene(CameraType cameraType,
                         vft::TessellationStrategy tessellationAlgorithm,
                         std::string font,
                         unsigned int fontSize,
                         bool useMsaa,
                         bool measureTime)
    : Scene{cameraType, tessellationAlgorithm, useMsaa, measureTime} {
    this->_font = std::make_shared<vft::Font>(font);
    if (tessellationAlgorithm == vft::TessellationStrategy::SDF) {
        vft::FontAtlas atlas{this->_font};
        this->_renderer->addFontAtlas(atlas);
    }

    this->_textBlock = vft::TextBlockBuilder()
                           .setWidth(this->_window->getWidth())
                           .setFont(this->_font)
                           .setFontSize(fontSize)
                           .setLineSpacing(1.2)
                           .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                           .setPosition(glm::vec3(0.f, 0.f, 0.f))
                           .build();
    this->_renderer->add(this->_textBlock);

    this->_window->setKeypressCallback([&](uint32_t codePoint) { this->_updateText(codePoint); });
}

EditorScene::~EditorScene() {}

void EditorScene::_updateText(uint32_t codePoint) {
    if (codePoint == vft::U_BACKSPACE) {
        this->_textBlock->remove();
    } else {
        this->_textBlock->add(std::u32string{codePoint});
    }
}
