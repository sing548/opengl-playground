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

    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
    pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    pi.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);

    Model model(ASSETS_DIR "/models/tie2/bland-tie.obj", pi, engine->GetAssMan());
    models.push_back(model);

    PhysicalInfo pi2 = PhysicalInfo();
    pi2.position_ = glm::vec3(-20.0f, 0.0f, 0.0f);
    pi2.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi2.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi2.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    pi2.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);

    Model model2(ASSETS_DIR "/models/tie2/bland-tie.obj", pi2, engine->GetAssMan());
    models.push_back(model2);

    engine->SetupScene(models);
    engine->Run();

    return 0;
}
