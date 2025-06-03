#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

#include "window/window.h"
#include "shaders/shader.h"
#include "models/model.h"
#include "models/scene.h"
#include "rendering/renderer.h"

// Window size
const unsigned int WIDTH = 1920;
const unsigned int HEIGHT = 1080;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    Window* window = nullptr;
    Renderer* renderer = nullptr;

    try {
        window = new Window(WIDTH, HEIGHT);
    }
    catch (const std::runtime_error e)
    {
        std::cerr << "Failed to initialize GLFW" << e.what() << std::endl;
        return -1;
    }

    renderer = new Renderer(WIDTH, HEIGHT);

    //Setup Scene
    Scene scene = Scene();
    Model model = Model("../assets/models/stars/generic_star/star.obj");
	////model_ = std::make_unique<Model>("../assets/models/landscape/test/default_landscape.obj");
    scene.AddModel(std::move(model));

    int i = 0;
    float accTime = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window->Get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;

        window->HandleInput(deltaTime);
        renderer->Draw(scene, window->GetCamera(), window->GetSize().width, window->GetSize().height);
        window->SwapBuffers();

        i++;
        if (i == 10000) {
            std::cout << "10000 frames done in: " << accTime << std::endl;
            i = 0;
            accTime = 0;
        }
    }

    delete window;
    return 0;
}
