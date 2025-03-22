/**
 * @file demo_scene.cpp
 * @author Christian Saloň
 */

#include "demo_scene.h"

const std::string DemoScene::JERSEY_PATH = "assets/Jersey10-Regular.ttf";
const std::string DemoScene::CRIMSON_TEXT_PATH = "assets/CrimsonText-Italic.ttf";
const std::string DemoScene::ROBOTO_PATH = "assets/Roboto-Regular.ttf";
const std::string DemoScene::ROBOTO_MONO_PATH = "assets/RobotoMono-Bold.ttf";
const std::string DemoScene::NOTO_SANS_JP_PATH = "assets/NotoSansJP-Regular.ttf";
const std::string DemoScene::NOTO_EMOJI_PATH = "assets/NotoEmoji.ttf";

const std::u8string DemoScene::ENGLISH_CODE_POINTS = u8"This is demo text";
const std::u16string DemoScene::SLOVAK_CODE_POINTS = u"Demonštračný text";
const std::u32string DemoScene::JAPANESE_CODE_POINTS = U"これはデモテキストです";
const std::u32string DemoScene::EMOJI_CODE_POINTS =
    {0x1F970, 0x1F480, 0x270C, 0x1F334, 0x1F422, 0x1F410, 0x1F344, 0x26BD, 0x1F37B, 0x1F451, 0x1F4F8};

DemoScene::DemoScene(CameraType cameraType, vft::TessellationStrategy tessellationAlgorithm, bool measureTime) : Scene{cameraType, tessellationAlgorithm, measureTime} {
    this->_jersey = std::make_shared<vft::Font>(DemoScene::JERSEY_PATH);
    this->_crimsontext = std::make_shared<vft::Font>(DemoScene::CRIMSON_TEXT_PATH);
    this->_roboto = std::make_shared<vft::Font>(DemoScene::ROBOTO_PATH);
    this->_robotomono = std::make_shared<vft::Font>(DemoScene::ROBOTO_MONO_PATH);
    this->_notosansjp = std::make_shared<vft::Font>(DemoScene::NOTO_SANS_JP_PATH);
    this->_notoemoji = std::make_shared<vft::Font>(DemoScene::NOTO_EMOJI_PATH);

    if (tessellationAlgorithm == vft::TessellationStrategy::SDF) {
        vft::FontAtlas jerseyAtlas{this->_jersey, vft::Unicode::utf8ToUtf32(ENGLISH_CODE_POINTS)};
        vft::FontAtlas crimsonTextAtlas{this->_crimsontext, vft::Unicode::utf8ToUtf32(ENGLISH_CODE_POINTS)};
        vft::FontAtlas robotoAtlas{this->_roboto, vft::Unicode::utf8ToUtf32(ENGLISH_CODE_POINTS).append(vft::Unicode::utf16ToUtf32(SLOVAK_CODE_POINTS))};
        vft::FontAtlas robotoMonoAtlas{this->_robotomono, vft::Unicode::utf8ToUtf32(ENGLISH_CODE_POINTS)};
        vft::FontAtlas notoSansJpAtlas{this->_notosansjp, JAPANESE_CODE_POINTS};
        vft::FontAtlas notoEmojiAtlas{this->_notoemoji, EMOJI_CODE_POINTS};

        this->_renderer->addFontAtlas(jerseyAtlas);
        this->_renderer->addFontAtlas(crimsonTextAtlas);
        this->_renderer->addFontAtlas(robotoAtlas);
        this->_renderer->addFontAtlas(robotoMonoAtlas);
        this->_renderer->addFontAtlas(notoSansJpAtlas);
        this->_renderer->addFontAtlas(notoEmojiAtlas);
    }

    this->_block1 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(64)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 0.f, 0.f))
        .setKerning(true)
        .build();

    this->_block2 = vft::TextBlockBuilder()
        .setFont(this->_roboto)
        .setFontSize(32)
        .setColor(glm::vec4(1.f, 0.f, 0.f, 1.f))
        .setPosition(glm::vec3(0.f, 64.f, 0.f))
        .setKerning(true)
        .build();

    this->_block3 = vft::TextBlockBuilder()
        .setFont(this->_robotomono)
        .setFontSize(32)
        .setColor(glm::vec4(0.f, 1.f, 0.f, 1.f))
        .setPosition(glm::vec3(0.f, 96.f, 0.f))
        .setKerning(true)
        .build();

    this->_block4 = vft::TextBlockBuilder()
        .setFont(this->_crimsontext)
        .setFontSize(32)
        .setColor(glm::vec4(0.f, 0.f, 1.f, 0.5f))
        .setPosition(glm::vec3(0.f, 128.f, 0.f))
        .setKerning(true)
        .build();

    this->_block5 = vft::TextBlockBuilder()
        .setFont(this->_jersey)
        .setFontSize(32)
        .setColor(glm::vec4(1.f, 1.f, 0.f, 1.f))
        .setPosition(glm::vec3(0.f, 160.f, 0.f))
        .setKerning(true)
        .build();

    this->_block6 = vft::TextBlockBuilder()
        .setFont(this->_notosansjp)
        .setFontSize(32)
        .setColor(glm::vec4(1.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 192.f, 0.f))
        .setKerning(false)
        .build();

    this->_block7 = vft::TextBlockBuilder()
        .setFont(this->_notoemoji)
        .setFontSize(32)
        .setColor(glm::vec4(0.f, 1.f, 1.f, 1.f))
        .setPosition(glm::vec3(0.f, 224.f, 0.f))
        .setKerning(false)
        .build();

    this->_renderer->add(this->_block1);
    this->_renderer->add(this->_block2);
    this->_renderer->add(this->_block3);
    this->_renderer->add(this->_block4);
    this->_renderer->add(this->_block5);
    this->_renderer->add(this->_block6);
    this->_renderer->add(this->_block7);

    this->_block1->add(DemoScene::SLOVAK_CODE_POINTS);
    this->_block2->add(DemoScene::ENGLISH_CODE_POINTS);
    this->_block3->add(DemoScene::ENGLISH_CODE_POINTS);
    this->_block4->add(DemoScene::ENGLISH_CODE_POINTS);
    this->_block5->add(DemoScene::ENGLISH_CODE_POINTS);
    this->_block6->add(DemoScene::JAPANESE_CODE_POINTS);
    this->_block7->add(DemoScene::EMOJI_CODE_POINTS);
}

DemoScene::~DemoScene() {}
