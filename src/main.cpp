#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

#include "window/window.h"
#include "shaders/shader.h"
#include "models/model.h"
#include "models/scene.h"

// Window size
const unsigned int WIDTH = 1920;
const unsigned int HEIGHT = 1080;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    Window* window = nullptr;

    try {
        window = new Window(WIDTH, HEIGHT);
    }
    catch (const std::runtime_error e)
    {
        std::cerr << "Failed to initialize GLFW" << e.what() << std::endl;
        return -1;
    }

    //Setup Scene

    Scene scene = Scene();

    Model model = Model("../assets/models/stars/generic_star/star.obj");
	////model_ = std::make_unique<Model>("../assets/models/landscape/test/default_landscape.obj");
    scene.AddModel(model);

    // Main loop
    while (!glfwWindowShouldClose(window->get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        window->processInput(deltaTime);
        window->draw(scene);
    }

    delete window;
    return 0;
}
