/**
 * @file main.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>

#include "demo_scene.h"
#include "base_camera.h"
#include "editor_scene.h"
#include "benchmark_scene.h"

/**
 * @brief Entry point for app
 */
int main(int argc, char **argv) {
    try {
        CameraType cameraType = CameraType::PERSPECTIVE;
        std::string sceneType = "demo";
        vft::TessellationStrategy tessellationAlgorithm = vft::TessellationStrategy::WINDING_NUMBER;
        bool measureTime = false;

        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], "-h") == 0) {
                // Show help message
                std::cout << "./vfont-demo [-h] [-c <perspective/orthographic>] [-s <demo/editor/benchmark>] -t" << std::endl;
                return EXIT_SUCCESS;
            }
            else if(strcmp(argv[i], "-c") == 0) {
                // Set camera type
                std::string type = argv[++i];

                if(type == "perspective") {
                    cameraType = CameraType::PERSPECTIVE;
                }
                else if(type == "orthographic") {
                    cameraType = CameraType::ORTHOGRAPHIC;
                }
                else {
                    std::cerr << "Camera type must be perspective or orthographic" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else if (strcmp(argv[i], "-s") == 0) {
                // Set scene type
                std::string type = argv[++i];

                if (type == "demo" || type == "editor" || type == "benchmark") {
                    sceneType = type;
                }
                else {
                    std::cerr << "Scene must be demo or editor or benchmark" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else if (strcmp(argv[i], "-a") == 0) {
                // Set tessellation algorithm
                std::string type = argv[++i];

                if (type == "cpu") {
                    tessellationAlgorithm = vft::TessellationStrategy::TRIANGULATION;
                }
                else if (type == "gpu") {
                    tessellationAlgorithm = vft::TessellationStrategy::WINDING_NUMBER;
                }
                else if (type == "combined") {
                    tessellationAlgorithm = vft::TessellationStrategy::TESSELLATION_SHADERS;
                }
                else {
                    std::cerr << "Tessellation algorithm must be cpu or gpu or combined" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else if (strcmp(argv[i], "-t") == 0) {
                // Set if measure time to render frame
                measureTime = true;
            }
            else {
                std::cerr << "Invalid argument at position " << i << std::endl;
                return EXIT_FAILURE;
            }
        }

        std::cout << "App started" << std::endl;
        if(sceneType == "editor") {
            EditorScene scene{cameraType, tessellationAlgorithm, measureTime};
            scene.run();
        }
        else if(sceneType == "demo") {
            DemoScene scene{cameraType, tessellationAlgorithm, measureTime};
            scene.run();
        } 
        else {
            BenchmarkScene scene{cameraType, tessellationAlgorithm, measureTime};
            scene.run();
        }
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "App closing because of error" << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "App closing" << std::endl;
    return EXIT_SUCCESS;
}
