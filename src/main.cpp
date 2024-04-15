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
    std::cout << "App started" << std::endl;

    try {
        CameraType cameraType = CameraType::PERSPECTIVE;
        std::string sceneType = "demo";

        for(int i = 1; i < argc; i++) {
            if(argv[i] == "-h") {
                // Show help message
                std::cout << "./kio [-h] [-c <perspective/ortographic>] [-s <demo/editor>]" << std::endl;
                return EXIT_SUCCESS;
            }
            else if(argv[i] == "-c") {
                // Set camera type
                std::string type = argv[++i];

                if(type == "perspective") {
                    cameraType = CameraType::PERSPECTIVE;
                }
                else if(type == "ortographic") {
                    cameraType = CameraType::ORTOGRAPHIC;
                }
                else {
                    std::cerr << "Camera type must be perspective or ortographic" << std::endl;
                    return EXIT_FAILURE;
                }
            }
            else if(argv[i] == "-s") {
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
