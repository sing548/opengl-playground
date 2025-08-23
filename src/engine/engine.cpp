#include "engine.h"

Engine::Engine()
{
    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, -90.0F);
    inputManager_ = std::make_unique<InputManager>();
    window_ = std::make_unique<Window>(WIDTH, HEIGHT, std::move(camera_), inputManager_.get());
    window_->setCallbacks();

    renderer_ = new Renderer(WIDTH, HEIGHT);
}

void Engine::SetupScene(std::vector<Model> models)
{
    scene_ = std::make_unique<Scene>();

    for (auto& m : models) {
        scene_->AddModel(m);    
    }
}

void Engine::Run()
{
    int i = 0;
    float accTime = 0.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    window_->setMouseLooking();

    while (!glfwWindowShouldClose(window_->Get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;

        window_->HandleInput(deltaTime);
        
        //HandleLogic();

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height);
        window_->SwapBuffers();

        i++;
        if (i == 10000) {
            std::cout << "10000 frames done in: " << accTime << std::endl;
            i = 0;
            accTime = 0;
        }
    }
    
    delete window_.get();
}

void Engine::HandleLogic()
{
    if (glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS) {
        Model& model = scene_->GetModelByReference(1);
        glm::vec3 position = model.GetPosition();
        position.z += 0.1f;
        model.SetPosition(position);
    }
}
