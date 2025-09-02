#include "engine.h"

Engine::Engine()
{
    //camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, -90.0F);
    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, 0.0F);
    inputManager_ = std::make_unique<InputManager>();
    window_ = std::make_unique<Window>(WIDTH, HEIGHT, std::move(camera_), inputManager_.get());
    window_->setCallbacks();

    renderer_ = std::make_unique<Renderer>(WIDTH, HEIGHT);

    settings_["adjust_camera"] = true;

    int i = 0;
    i++;
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

    //window_->setMouseLooking();

    while (!glfwWindowShouldClose(window_->Get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;

        //window_->HandleInput(deltaTime);
        
        HandleInput(deltaTime);
        HandleLogic(deltaTime);

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height);
        window_->SwapBuffers();

        i++;
        if (i == 1200) {
            std::cout << "1200 frames done in: " << accTime << std::endl;
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
    /*auto currentPosition = window_->GetCamera().Position;
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

    // Update camera position (looking straight down)
    window_->UpdateCameraPosition(glm::vec3(0, h, 0));*/

    /*auto model = scene_->GetModelByReference(1);
    auto pos = model.GetPosition();

    window_->UpdateCameraPosition(pos + model.GetOrientation() * glm::vec3(5.0f, 5.0f, 5.0f));

    auto pos2 = model.GetPosition();
    pos2 -= model.GetOrientation() * glm::vec3(5.0f, 5.0f, 5.0f);
    window_->UpdateCameraOrientation(pos2);*/


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
}

void Engine::Shoot(Model shooter)
{

    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = shooter.GetPosition();
    pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi.orientation_ = shooter.GetOrientation();
    pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi.speed_ = shooter.GetSpeed() + glm::vec3(0.1f, 0.0f, 0.0f);

    Model shot(0.1f, pi);

    shot.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
    scene_->AddModel(shot);
}

void Engine::MoveModel(unsigned int id, glm::vec3 change)
{
    Model& model = scene_->GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += change;

    /*if (id == 1 || id == 2) {
        if (abs(position.x) > 20) {
            position.x *=  -1;
        }
        
        if (abs(position.z) > 15) {
            position.z *= -1;
        }
    }*/

    model.SetPosition(position);

    //if (abs(position.x) > this->currentFurthestPosition.x) this->currentFurthestPosition.x = abs(position.x);
    //if (abs(position.z) > this->currentFurthestPosition.z) this->currentFurthestPosition.z = abs(position.z);
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

    //for (auto id : deletes) 
    //    scene_->RemoveModel(id);
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
        RotateModel(glm::vec3(0.0f, glm::radians(1.5f), 0.0f));
    }

    if (glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS) {
        RotateModel(glm::vec3(0.0f, glm::radians(-1.5f), 0.0f));
    }
    
    // Flight logic
    if (glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS) {
        float acc = deltaTime * 0.1f;
        glm::vec3 speed = scene_->GetModelByReference(1).GetSpeed();
        speed.x += acc;

        if (1.0f > speed.x)
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

    // Hitboxes
    if (glfwGetKey(window_->Get(), GLFW_KEY_F8) == GLFW_PRESS) {
        renderer_->ToggleHitboxes();
    }

    // Shooting
    if (glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS) {
        Shoot(scene_->GetModelByReference(1));
    }
}
