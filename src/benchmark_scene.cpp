/**
 * @file benchmark_scene.cpp
 * @author Christian Saloň
 */

#include <VFONT/timed_renderer.h>

#include "benchmark_scene.h"

const std::string BenchmarkScene::ROBOTO_PATH = "assets/Roboto-Regular.ttf";

const std::string BenchmarkScene::CODE_POINTS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

BenchmarkScene::BenchmarkScene(CameraType cameraType, vft::Renderer::TessellationStrategy tessellationAlgorithm, bool measureTime) : Scene{cameraType, tessellationAlgorithm, measureTime} {
    this->_roboto = std::make_shared<vft::Font>(BenchmarkScene::ROBOTO_PATH);

    this->_block1 = std::make_shared<vft::TextBlock>(this->_roboto, 256, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1, false, false);
    this->_block2 = std::make_shared<vft::TextBlock>(this->_roboto, 128, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 256.f, 0.f), -1, false, false);
    this->_block3 = std::make_shared<vft::TextBlock>(this->_roboto, 64, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 384.f, 0.f), -1, false, false);
    this->_block4 = std::make_shared<vft::TextBlock>(this->_roboto, 32, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 448.f, 0.f), -1, false, false);
    this->_block5 = std::make_shared<vft::TextBlock>(this->_roboto, 16, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 480.f, 0.f), -1, false, false);
    this->_block6 = std::make_shared<vft::TextBlock>(this->_roboto, 8, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 496.f, 0.f), -1, false, false);

    this->_renderer->add(this->_block1);
    this->_renderer->add(this->_block2);
    this->_renderer->add(this->_block3);
    this->_renderer->add(this->_block4);
    this->_renderer->add(this->_block5);
    this->_renderer->add(this->_block6);

    this->_block1->add(BenchmarkScene::CODE_POINTS);
    this->_block2->add(BenchmarkScene::CODE_POINTS);
    this->_block3->add(BenchmarkScene::CODE_POINTS);
    this->_block4->add(BenchmarkScene::CODE_POINTS);
    this->_block5->add(BenchmarkScene::CODE_POINTS);
    this->_block6->add(BenchmarkScene::CODE_POINTS);
}

BenchmarkScene::~BenchmarkScene() {}
