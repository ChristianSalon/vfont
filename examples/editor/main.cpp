/**
 * @file main.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>

#include <VFONT/text_renderer.h>

#include "base_camera.h"
#include "editor_scene.h"

int main(int argc, char **argv) {
    try {
        CameraType cameraType = CameraType::PERSPECTIVE;
        vft::TessellationStrategy tessellationAlgorithm = vft::TessellationStrategy::WINDING_NUMBER;
        bool measureTime = false;

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0) {
                // Show help message
                std::cout << "./demo [-h] [-c <perspective/orthographic>] [-a <cdt/ts/wn/sdf>] -t" << std::endl;
                return EXIT_SUCCESS;
            } else if (strcmp(argv[i], "-c") == 0) {
                // Set camera type
                std::string type = argv[++i];

                if (type == "perspective") {
                    cameraType = CameraType::PERSPECTIVE;
                } else if (type == "orthographic") {
                    cameraType = CameraType::ORTHOGRAPHIC;
                } else {
                    std::cerr << "Camera type must be perspective or orthographic" << std::endl;
                    return EXIT_FAILURE;
                }
            } else if (strcmp(argv[i], "-a") == 0) {
                // Set tessellation algorithm
                std::string type = argv[++i];

                if (type == "cdt") {
                    tessellationAlgorithm = vft::TessellationStrategy::TRIANGULATION;
                } else if (type == "wn") {
                    tessellationAlgorithm = vft::TessellationStrategy::WINDING_NUMBER;
                } else if (type == "ts") {
                    tessellationAlgorithm = vft::TessellationStrategy::TESSELLATION_SHADERS;
                } else if (type == "sdf") {
                    tessellationAlgorithm = vft::TessellationStrategy::SDF;
                } else {
                    std::cerr << "Tessellation algorithm must be cdt, ts, wn or sdf" << std::endl;
                    return EXIT_FAILURE;
                }
            } else if (strcmp(argv[i], "-t") == 0) {
                // Set if measure time to render frame
                measureTime = true;
            } else {
                std::cerr << "Invalid argument at position " << i << std::endl;
                return EXIT_FAILURE;
            }
        }

        EditorScene scene{cameraType, tessellationAlgorithm, measureTime};
        scene.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
