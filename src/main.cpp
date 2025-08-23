#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

#include "window/window.h"
#include "shaders/shader.h"
#include "models/model.h"
#include "models/scene.h"
#include "rendering/renderer.h"
#include "engine/engine.h"

#ifndef ASSETS_DIR
#define ASSETS_DIR "./assets"
#endif

int main() {
    Engine* engine = nullptr;

    try {
        engine = new Engine();
    }
    catch (const std::runtime_error e)
    {
        std::cerr << "Failed to initialize GLFW" << e.what() << std::endl;
        return -1;
    }

    std::vector<Model> models = std::vector<Model>();

    Model model(ASSETS_DIR "/models/tie/tie.obj");
    models.push_back(model);

    engine->SetupScene(models);
    engine->Run();
    
    return 0;
}
