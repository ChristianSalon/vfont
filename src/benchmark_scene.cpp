/**
 * @file benchmark_scene.cpp
 * @author Christian Saloň
 */

#include "benchmark_scene.h"

const std::string BenchmarkScene::ROBOTO_PATH = "assets/Roboto-Regular.ttf";

const std::u8string BenchmarkScene::TEXT = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

BenchmarkScene::BenchmarkScene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool measureTime) : Scene{cameraType, tessellationAlgorithm, measureTime} {
    // this->_renderer->setCacheSize(0);

    this->_roboto = std::make_shared<vft::Font>(BenchmarkScene::ROBOTO_PATH);

    this->_block1 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(256)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 0.f, 0.f))
        .setKerning(true)
        .build();

    this->_block2 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(128)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 256.f, 0.f))
        .setKerning(true)
        .build();

    this->_block3 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(64)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 384.f, 0.f))
        .setKerning(true)
        .build();

    this->_block4 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(32)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 448.f, 0.f))
        .setKerning(true)
        .build();

    this->_block5 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(16)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 480.f, 0.f))
        .setKerning(true)
        .build();

    this->_block6 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(8)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 496.f, 0.f))
        .setKerning(true)
        .build();

    this->_block7 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(256)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 504.f, 0.f))
        .setKerning(true)
        .build();

    this->_block8 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(128)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 760.f, 0.f))
        .setKerning(true)
        .build();

    this->_block9 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(64)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 888.f, 0.f))
        .setKerning(true)
        .build();

    this->_block10 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(32)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 952.f, 0.f))
        .setKerning(true)
        .build();

    this->_block11 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(16)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 984.f, 0.f))
        .setKerning(true)
        .build();

    this->_block12 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(8)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 1000.f, 0.f))
        .setKerning(true)
        .build();

    this->_renderer->add(this->_block1);
    this->_renderer->add(this->_block2);
    this->_renderer->add(this->_block3);
    this->_renderer->add(this->_block4);
    this->_renderer->add(this->_block5);
    this->_renderer->add(this->_block6);

    this->_renderer->add(this->_block7);
    this->_renderer->add(this->_block8);
    this->_renderer->add(this->_block9);
    this->_renderer->add(this->_block10);
    this->_renderer->add(this->_block11);
    this->_renderer->add(this->_block12);

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10; i++) {
        this->_block1->add(BenchmarkScene::TEXT);
        this->_block2->add(BenchmarkScene::TEXT);
        this->_block3->add(BenchmarkScene::TEXT);
        this->_block4->add(BenchmarkScene::TEXT);
        this->_block5->add(BenchmarkScene::TEXT);
        this->_block6->add(BenchmarkScene::TEXT);

        this->_block7->add(BenchmarkScene::TEXT);
        this->_block8->add(BenchmarkScene::TEXT);
        this->_block9->add(BenchmarkScene::TEXT);
        this->_block10->add(BenchmarkScene::TEXT);
        this->_block11->add(BenchmarkScene::TEXT);
        this->_block12->add(BenchmarkScene::TEXT);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Tessellation time: " << time << " milliseconds" << std::endl;
}

BenchmarkScene::~BenchmarkScene() {}
