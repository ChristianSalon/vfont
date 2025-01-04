/**
 * @file benchmark_scene.h
 * @author Christian Saloň
 */

#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <iostream>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "scene.h"

 /**
  * @class BenchmarkScene
  */
class BenchmarkScene : public Scene {

public:

	static const std::string ROBOTO_PATH;
	static const std::string CODE_POINTS;

private:

	std::shared_ptr<vft::Font> _roboto;

	std::shared_ptr<vft::TextBlock> _block1;
	std::shared_ptr<vft::TextBlock> _block2;
	std::shared_ptr<vft::TextBlock> _block3;
	std::shared_ptr<vft::TextBlock> _block4;
	std::shared_ptr<vft::TextBlock> _block5;
	std::shared_ptr<vft::TextBlock> _block6;

	std::shared_ptr<vft::TextBlock> _block7;
	std::shared_ptr<vft::TextBlock> _block8;
	std::shared_ptr<vft::TextBlock> _block9;
	std::shared_ptr<vft::TextBlock> _block10;
	std::shared_ptr<vft::TextBlock> _block11;
	std::shared_ptr<vft::TextBlock> _block12;

public:

	BenchmarkScene(CameraType cameraType, vft::Renderer::TessellationStrategy tessellationAlgorithm, bool measureTime = true);
	~BenchmarkScene();

};
