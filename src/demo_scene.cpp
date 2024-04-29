/**
 * @file demo_scene.cpp
 * @author Christian Saloň
 */

#include "demo_scene.h"

const std::string DemoScene::ARIAL_PATH = "Arial-Regular.ttf";
const std::string DemoScene::JERSEY_PATH = "Jersey10-Regular.ttf";
const std::string DemoScene::CRIMSON_TEXT_PATH = "CrimsonText-Italic.ttf";
const std::string DemoScene::ROBOTO_MONO_PATH = "RobotoMono-Bold.ttf";
const std::string DemoScene::NOTO_SANS_JP_PATH = "NotoSansJP-Regular.ttf";

const std::vector<uint32_t> DemoScene::ENGLISH_CODE_POINTS =
    {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x64, 0x65, 0x6D, 0x6F, 0x20, 0x74, 0x65, 0x78, 0x74};
const std::vector<uint32_t> DemoScene::SLOVAK_CODE_POINTS =
    {0x0044, 0x0065, 0x006d, 0x006f, 0x006e, 0x0161, 0x0074, 0x0072, 0x0061, 0x010d, 0x006e, 0x00fd, 0x0020, 0x0074, 0x0065, 0x0078, 0x0074};
const std::vector<uint32_t> DemoScene::JAPANESE_CODE_POINTS =
    {0x3053, 0x308C, 0x306F, 0x30C7, 0x30E2, 0x30C6, 0x30AD, 0x30B9, 0x30C8, 0x3067, 0x3059};

DemoScene::DemoScene(CameraType cameraType) : Scene{cameraType} {
    this->_arial32 = std::make_shared<vft::Font>(DemoScene::ARIAL_PATH, 32);
    this->_arial64 = std::make_shared<vft::Font>(DemoScene::ARIAL_PATH, 64);
    this->_jersey32 = std::make_shared<vft::Font>(DemoScene::JERSEY_PATH, 128);
    this->_crimsontext32 = std::make_shared<vft::Font>(DemoScene::CRIMSON_TEXT_PATH, 32);
    this->_robotomono32 = std::make_shared<vft::Font>(DemoScene::ROBOTO_MONO_PATH, 32);
    this->_notosansjp32 = std::make_shared<vft::Font>(DemoScene::NOTO_SANS_JP_PATH, 32);

    this->_block1 = std::make_shared<vft::TextBlock>(this->_arial64, glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 0.f, 0.f), -1);
    this->_block1->add(DemoScene::SLOVAK_CODE_POINTS);

    this->_block2 = std::make_shared<vft::TextBlock>(this->_arial32, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 64.f, 0.f), -1);
    this->_block2->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block5 = std::make_shared<vft::TextBlock>(this->_jersey32, glm::vec3(1.f, 1.f, 0.f), glm::vec3(0.f, 160.f, 0.f), -1);
    this->_block5->add(DemoScene::ENGLISH_CODE_POINTS);

    /*this->_block3 = std::make_shared<vft::TextBlock>(this->_robotomono32, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 96.f, 0.f), -1, false, false);
    this->_block3->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block4 = std::make_shared<vft::TextBlock>(this->_crimsontext32, glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 128.f, 0.f), -1);
    this->_block4->add(DemoScene::ENGLISH_CODE_POINTS);

    this->_block6 = std::make_shared<vft::TextBlock>(this->_notosansjp32, glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, 192.f, 0.f), -1);
    this->_block6->add({0x308C});*/

    this->renderer.add(this->_block1);
    this->renderer.add(this->_block2);
    this->renderer.add(this->_block5);
    /*this->renderer.add(this->_block3);
    this->renderer.add(this->_block4);
    this->renderer.add(this->_block6);*/
}

DemoScene::~DemoScene() {}
