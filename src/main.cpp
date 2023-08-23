/**
 * @file main.cpp
 * @author Christian Saloň
 */

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "window.h"

class App {

private:

    Window *window = nullptr;

public:

    void run() {
        initVulkan();
        createWindow();
        mainLoop();
        cleanup();
    }

private:

    void initVulkan() {}

    void createWindow() {
        this->window = new Window;
        this->window->init();
        this->window->show();
    }

    void mainLoop() {
        this->window->mainLoop();
    }

    void cleanup() {
        delete this->window;
        this->window = nullptr;
    }
};

int main() {
    std::cout << "App started" << std::endl;

    App app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "App closing because of error" << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "App closing" << std::endl;
    return EXIT_SUCCESS;
}
