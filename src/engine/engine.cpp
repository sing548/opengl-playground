#include "engine.h"

#ifndef ASSETS_DIR
#define ASSETS_DIR "./assets"
#endif

Engine::Engine()
{
    bool adjust = true;
    settings_["adjust_camera"] = adjust;
    bool mouseLooking = false;
    settings_["mouse_looking"] = mouseLooking;
    bool thirdPerson = false;
    settings_["third_person"] = thirdPerson;
    bool skyBox = false;
    settings_["sky_box"] = skyBox;
    bool hitboxes = false;
    settings_["hitboxes"] = hitboxes;
    bool bloom = true;
    settings_["bloom"] = bloom;

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, -90.0F);
    
    inputManager_ = std::make_unique<InputManager>();
    window_ = std::make_unique<Window>(WIDTH, HEIGHT, std::move(camera_), inputManager_.get());
    window_->setCallbacks();
    renderer_ = std::make_unique<Renderer>(WIDTH, HEIGHT, hitboxes, skyBox);
    assMan_ = std::make_unique<AssetManager>();

    glfwSetWindowUserPointer(window_->Get(), this);
    glfwSetKeyCallback(window_->Get(), Engine::KeyCallback);

    glfwSetCursorPosCallback(window_->Get(), [](GLFWwindow* window, double xPos, double yPos) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

        if (engine && engine->settings_["mouse_looking"]) {
            engine->window_->mouse_callback(xPos, yPos);
        }
    });
}

AssetManager& Engine::GetAssMan()
{
    return *assMan_;
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

    while (!glfwWindowShouldClose(window_->Get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;
        
        glfwPollEvents();
        HandleInput(deltaTime);
        HandleLogic(deltaTime);

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height, settings_);
        window_->SwapBuffers();

        i++;
        if (accTime >= 1) {
            std::cout << "Current FPS: " << i / accTime << std::endl;
            i = 0;
            accTime = 0;
        }
    }
    
    delete window_.get();
}

void Engine::HandleLogic(float deltaTime)
{
    MoveModels();
    
    bool adjust = settings_["adjust_camera"];

    if (adjust)
        AdjustCamera();
    
    CheckHits();
}

void Engine::CheckHits()
{
    auto model1 = scene_->GetModelByReference(1);
    auto model2 = scene_->GetModelByReference(2);

    for (auto it : scene_->GetModels()) {
        if (it.Id == 1 || it.Id == 2)
            continue;

        float distance = glm::length(it.model.GetPosition() - model2.GetPosition());
        float radius = it.model.radius + model2.radius;
        
        if (distance <= radius) {
           glfwSetWindowShouldClose(window_->Get(), true);
        }
    }
}

void Engine::AdjustCamera()
{
    if (settings_["third_person"]) {
        auto model = scene_->GetModelByReference(1);
        glm::vec3 modelPos = model.GetPosition();
        
        // Offset from the model in its local orientation
        glm::vec3 offset = model.GetOrientation() * glm::vec3(-8.0f, -8.0f, -8.0f);
        glm::vec3 cameraPos = modelPos + offset + glm::vec3(0.0f, 1.5f, 0.0f);
    
        // Update camera position
        window_->UpdateCameraPosition(cameraPos);
    
        // Compute direction from camera to model
        glm::vec3 front = glm::normalize(modelPos - cameraPos);
        window_->UpdateCameraOrientation(front);
    } else {
        auto currentPosition = window_->GetCamera().Position;
        float minHeight = 20.0f;
        float maxHeight = 100.0f;

        float fovyRadians = glm::radians(window_->GetCamera().Zoom);
        float aspect = (float) WIDTH/ (float)HEIGHT;

        float halfFovTan = tanf(fovyRadians * 0.5f);

        // Required heights in each axis
        float requiredX = (abs(currentFurthestPosition.x) + 1.5f) / (halfFovTan * aspect);
        float requiredZ = (abs(currentFurthestPosition.z) + 1.5f)/ halfFovTan;

        // Take the larger
        float h = std::max(requiredX, requiredZ);

        // Clamp
        h = std::clamp(h, minHeight, maxHeight);

        float oldHeight = window_->GetCamera().Position.y;

        float changeRate = 0.25f;

        if (abs(h - oldHeight) > changeRate) {
            if (h > oldHeight) h = oldHeight + changeRate;
            if (h < oldHeight) h = oldHeight - changeRate;
        }

        window_->UpdateCameraPosition(glm::vec3(0, h, 0));
    }
}

void Engine::Shoot(Model shooter)
{
    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = shooter.GetPosition();
    pi.rotation_ = shooter.GetRotation();
    pi.scale_ = glm::vec3(0.05f, 0.05f, 0.05f);
    pi.orientation_ = shooter.GetOrientation();
    pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi.speed_ = shooter.GetSpeed() + glm::vec3(0.1f, 0.0f, 0.0f);

    Model shot(ASSETS_DIR "/models/shot/shot.obj", pi, *assMan_, true, 0.05f);

    scene_->AddModel(shot);
}

void Engine::MoveModel(unsigned int id, glm::vec3 change)
{
    Model& model = scene_->GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += change;
    model.SetPosition(position);

    if (abs(position.x) > this->currentFurthestPosition.x) this->currentFurthestPosition.x = abs(position.x);
    if (abs(position.z) > this->currentFurthestPosition.z) this->currentFurthestPosition.z = abs(position.z);
}

void Engine::MoveModels()
{
    std::vector<unsigned int> deletes;
             
    currentFurthestPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    for (auto it : scene_->GetModels()) {
        glm::vec3 change = it.model.GetOrientation() * it.model.GetSpeed().x;
        MoveModel(it.Id, change);

        auto position = it.model.GetPosition();
        if (abs(position.x) > 80 || abs(position.z) > 80)
            deletes.push_back(it.Id);
    }

    for (auto id : deletes) {
        if (id != 1 && id != 2)
            scene_->RemoveModel(id);
    }
}

void Engine::RotateModel(glm::vec3 change) 
{
    Model& model = scene_->GetModelByReference(1);
    glm::vec3 rotation = model.GetRotation();
    rotation += change;
    model.SetRotation(rotation);
}

void Engine::ChangeSetting(std::string key, bool value)
{
    settings_[key] = value;
}

void Engine::HandleInput(float deltaTime)
{
    if (glfwGetKey(window_->Get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_->Get(), true);
    }

    if (glfwGetKey(window_->Get(), GLFW_KEY_A) == GLFW_PRESS) {
        RotateModel(glm::vec3(0.0f, glm::radians(0.7f), 0.0f));
    }

    if (glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS) {
        RotateModel(glm::vec3(0.0f, glm::radians(-0.7f), 0.0f));
    }
    
    // Flight logic
    if (glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS) {
        float acc = deltaTime * 0.05f;
        glm::vec3 speed = scene_->GetModelByReference(1).GetSpeed();
        speed.x += acc;

        // Max Speed
        if (0.1f > speed.x)
            scene_->GetModelByReference(1).SetSpeed(speed);
    } else if (glfwGetKey(window_->Get(), GLFW_KEY_SPACE) != GLFW_PRESS) {
        float acc = deltaTime * 0.05f;

        if (glfwGetKey(window_->Get(), GLFW_KEY_S) == GLFW_PRESS)
            acc = deltaTime * .2f;

        glm::vec3 speed = scene_->GetModelByReference(1).GetSpeed();
        
        if (speed.x > 0) {
            speed.x -= acc;
        }
        if (speed.x < 0) speed.x = 0;
        scene_->GetModelByReference(1).SetSpeed(speed);
    }

    if (lastShot > 0)
        lastShot -= deltaTime;

    if (lastShot < 0)
        lastShot = 0;

    // Shooting
    if (glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS && lastShot == 0 && shotReleased) {
        Shoot(scene_->GetModelByReference(1));
        lastShot = 5;
        shotReleased = false;
    }

    if (glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_RELEASE) {
        shotReleased = true;
    }
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine) {
    // Hitboxes
        if (key == GLFW_KEY_F8 && action == GLFW_PRESS) {
            engine->renderer_->ToggleHitboxes();
        }

        if (key == GLFW_KEY_F9 && action == GLFW_PRESS) {
            engine->renderer_->ToggleSkyBox();
        }

        if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
            engine->settings_["adjust_camera"] = !engine->settings_["adjust_camera"];
        }

        if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
            engine->settings_["third_person"] = !engine->settings_["third_person"];

            if (engine->settings_["third_person"]) {
                engine->settings_["adjust_camera"] = true;
                auto& cam = engine->window_->GetCamera();
                cam.Up = cam.WorldUp;
                cam.Pitch = 0.0f;
                cam.UpdateCameraVectors();
            } else {
                auto& cam = engine->window_->GetCamera();
                cam.Up = glm::vec3(0.0f, 0.0f, 1.0f);
                cam.Front = glm::vec3(0.0f, 0.0f, -1.0f);
                cam.Pitch = -90.0f;
                cam.UpdateCameraVectors();
                cam.Position = glm::vec3(0.0f, 60.0f, 0.0f);
            }
        }

        if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
            engine->settings_["bloom"] = !engine->settings_["bloom"];
        }
    }
}
