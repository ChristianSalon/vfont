/**
 * @file main.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>

#include <VFONT/text_renderer.h>

#include "base_camera.h"
#include "demo_scene.h"

int main(int argc, char **argv) {
    try {
        CameraType cameraType = CameraType::PERSPECTIVE;
        vft::TessellationStrategy tessellationAlgorithm = vft::TessellationStrategy::SDF;
        bool measureTime = false;
        bool useMsaa = false;

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0) {
                // Show help message
                std::cout << "./demo [-h] [-c <perspective/orthographic>] [-a <cdt/ts/wn/sdf>] [-t] [-m]" << std::endl;
                std::cout << "-h: Show help message" << std::endl;
                std::cout << "-c: Select the type of camera used" << std::endl;
                std::cout << "-a: Select the rendering algorithm" << std::endl;
                std::cout << "  cdt - Constrained delaunay triangulation on the cpu" << std::endl;
                std::cout << "  ts - Outer triangles processed by tessellation shaders, inner triangulated on the cpu"
                          << std::endl;
                std::cout << "  wn - Winding number calculated in fragment shader" << std::endl;
                std::cout << "  sdf - Signed distance field" << std::endl;
                std::cout << "-t: Measure the gpu draw time" << std::endl;
                std::cout << "-m: Use multisampling antialiasing" << std::endl;
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
            } else if (strcmp(argv[i], "-m") == 0) {
                // Set multisampling
                useMsaa = true;
            } else {
                std::cerr << "Invalid argument at position " << i << std::endl;
                return EXIT_FAILURE;
            }
        }

        DemoScene scene{cameraType, tessellationAlgorithm, useMsaa, measureTime};
        scene.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
