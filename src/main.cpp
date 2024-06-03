/**
 * @file main.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>

#include "demo_scene.h"
#include "base_camera.h"
#include "editor_scene.h"

/**
 * @brief Entry point for app
 */
int main(int argc, char **argv) {
    try {
        CameraType cameraType = CameraType::PERSPECTIVE;
        std::string sceneType = "demo";

        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], "-h") == 0) {
                // Show help message
                std::cout << "./kio [-h] [-c <perspective/orthographic>] [-s <demo/editor>]" << std::endl;
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
            else if(strcmp(argv[i], "-s") == 0) {
                // Set scene type
                std::string type = argv[++i];

                if(type == "demo" || type == "editor") {
                    sceneType = type;
                }
                else {
                    std::cerr << "Scene must be demo or editor" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else {
                std::cerr << "Invalid argument at position " << i << std::endl;
                return EXIT_FAILURE;
            }
        }

        std::cout << "App started" << std::endl;
        if(sceneType == "editor") {
            EditorScene scene{cameraType};
            scene.run();
        }
        else {
            DemoScene scene{cameraType};
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
