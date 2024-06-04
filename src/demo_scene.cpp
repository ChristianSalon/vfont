/**
 * @file demo_scene.cpp
 * @author Christian Saloň
 */

#include "demo_scene.h"

#include <glm/vec4.hpp>

const std::string DemoScene::JERSEY_PATH = "assets/Jersey10-Regular.ttf";
const std::string DemoScene::CRIMSON_TEXT_PATH = "assets/CrimsonText-Italic.ttf";
const std::string DemoScene::ROBOTO_PATH = "assets/Roboto-Regular.ttf";
const std::string DemoScene::ROBOTO_MONO_PATH = "assets/RobotoMono-Bold.ttf";
const std::string DemoScene::NOTO_SANS_JP_PATH = "assets/NotoSansJP-Regular.ttf";
const std::string DemoScene::NOTO_EMOJI_PATH = "assets/NotoEmoji.ttf";

const std::vector<uint32_t> DemoScene::ENGLISH_CODE_POINTS =
    {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x64, 0x65, 0x6D, 0x6F, 0x20, 0x74, 0x65, 0x78, 0x74};
const std::vector<uint32_t> DemoScene::SLOVAK_CODE_POINTS =
    {0x0044, 0x0065, 0x006d, 0x006f, 0x006e, 0x0161, 0x0074, 0x0072, 0x0061, 0x010d, 0x006e, 0x00fd, 0x0020, 0x0074, 0x0065, 0x0078, 0x0074};
const std::vector<uint32_t> DemoScene::JAPANESE_CODE_POINTS =
    {0x3053, 0x308C, 0x306F, 0x30C7, 0x30E2, 0x30C6, 0x30AD, 0x30B9, 0x30C8, 0x3067, 0x3059};
const std::vector<uint32_t> DemoScene::EMOJI_CODE_POINTS =
    {0x1F970, 0x1F480, 0x270C, 0xFE0F, 0x1F334, 0x1F422, 0x1F410, 0x1F344, 0x26BD, 0x1F37B, 0x1F451, 0x1F4F8};

DemoScene::DemoScene(CameraType cameraType) : Scene{cameraType} {
    this->_jersey = std::make_shared<vft::Font>(DemoScene::JERSEY_PATH);
    this->_crimsontext = std::make_shared<vft::Font>(DemoScene::CRIMSON_TEXT_PATH);
    this->_roboto = std::make_shared<vft::Font>(DemoScene::ROBOTO_PATH);
    this->_robotomono = std::make_shared<vft::Font>(DemoScene::ROBOTO_MONO_PATH);
    this->_notosansjp = std::make_shared<vft::Font>(DemoScene::NOTO_SANS_JP_PATH);
    this->_notoemoji = std::make_shared<vft::Font>(DemoScene::NOTO_EMOJI_PATH);

    this->_block1 = std::make_shared<vft::TextBlock>(this->_roboto, 64, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1, false, false);
    this->_block1->add(DemoScene::SLOVAK_CODE_POINTS);

    this->_block2 = std::make_shared<vft::TextBlock>(this->_roboto, 32, glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec3(0.f, 64.f, 0.f), -1);
    this->_block2->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block3 = std::make_shared<vft::TextBlock>(this->_robotomono, 32, glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec3(0.f, 96.f, 0.f), -1, false, false);
    this->_block3->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block4 = std::make_shared<vft::TextBlock>(this->_crimsontext, 32, glm::vec4(0.f, 0.f, 1.f, 0.5f), glm::vec3(0.f, 128.f, 0.f), -1, false, false);
    this->_block4->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block5 = std::make_shared<vft::TextBlock>(this->_jersey, 32, glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec3(0.f, 160.f, 0.f), -1);
    this->_block5->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block6 = std::make_shared<vft::TextBlock>(this->_notosansjp, 32, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 192.f, 0.f), -1, false, false);
    this->_block6->add(DemoScene::JAPANESE_CODE_POINTS);

    this->_block7 = std::make_shared<vft::TextBlock>(this->_notoemoji, 32, glm::vec4(0.f, 1.f, 1.f, 1.f), glm::vec3(0.f, 224.f, 0.f), -1, false, false);
    this->_block7->add(DemoScene::EMOJI_CODE_POINTS);

    this->_renderer.add(this->_block1);
    this->_renderer.add(this->_block2);
    this->_renderer.add(this->_block3);
    this->_renderer.add(this->_block4);
    this->_renderer.add(this->_block5);
    this->_renderer.add(this->_block6);
    this->_renderer.add(this->_block7);
}

DemoScene::~DemoScene() {}
