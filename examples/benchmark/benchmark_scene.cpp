/**
 * @file benchmark_scene.cpp
 * @author Christian Saloň
 */

#include "benchmark_scene.h"

const std::u8string BenchmarkScene::TEXT = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

BenchmarkScene::BenchmarkScene(CameraType cameraType,
                               vft::TessellationStrategy tessellationAlgorithm,
                               std::string font,
                               bool measureTime)
    : Scene{cameraType, tessellationAlgorithm, measureTime} {
    // this->_renderer->setCacheSize(0);

    this->_font = std::make_shared<vft::Font>(font);

    this->_block1 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(256)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 0.f, 0.f))
                        .build();

    this->_block2 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(128)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 256.f, 0.f))
                        .build();

    this->_block3 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(64)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 384.f, 0.f))
                        .build();

    this->_block4 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(32)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 448.f, 0.f))
                        .build();

    this->_block5 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(16)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 480.f, 0.f))
                        .build();

    this->_block6 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(8)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 496.f, 0.f))
                        .build();

    this->_block7 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(256)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 504.f, 0.f))
                        .build();

    this->_block8 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(128)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 760.f, 0.f))
                        .build();

    this->_block9 = vft::TextBlockBuilder()
                        .setFont(this->_font)
                        .setFontSize(64)
                        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                        .setPosition(glm::vec3(0.f, 888.f, 0.f))
                        .build();

    this->_block10 = vft::TextBlockBuilder()
                         .setFont(this->_font)
                         .setFontSize(32)
                         .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                         .setPosition(glm::vec3(0.f, 952.f, 0.f))
                         .build();

    this->_block11 = vft::TextBlockBuilder()
                         .setFont(this->_font)
                         .setFontSize(16)
                         .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                         .setPosition(glm::vec3(0.f, 984.f, 0.f))
                         .build();

    this->_block12 = vft::TextBlockBuilder()
                         .setFont(this->_font)
                         .setFontSize(8)
                         .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
                         .setPosition(glm::vec3(0.f, 1000.f, 0.f))
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

    if (tessellationAlgorithm == vft::TessellationStrategy::SDF) {
        vft::FontAtlas atlas{this->_font, vft::Unicode::utf8ToUtf32(TEXT)};
        this->_renderer->addFontAtlas(atlas);
    }

    for (int i = 0; i < 10; i++) {
        this->_block1->add(TEXT);
        this->_block2->add(TEXT);
        this->_block3->add(TEXT);
        this->_block4->add(TEXT);
        this->_block5->add(TEXT);
        this->_block6->add(TEXT);

        this->_block7->add(TEXT);
        this->_block8->add(TEXT);
        this->_block9->add(TEXT);
        this->_block10->add(TEXT);
        this->_block11->add(TEXT);
        this->_block12->add(TEXT);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Tessellation time: " << time << " milliseconds" << std::endl;
}

BenchmarkScene::~BenchmarkScene() {}
