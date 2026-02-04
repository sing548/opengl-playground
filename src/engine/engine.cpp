#include "engine.h"

#ifndef ASSETS_DIR
#define ASSETS_DIR "./assets"
#endif

Engine::Engine(int config)
{
    if (config == 1)
    {
        m_bNetworking = true;
        m_bServer = true;
    }
    else if (config == 2)
    {
        m_bNetworking = true;
    }

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

    if (m_bNetworking)
    {
        playerId_ = m_bServer ? 1 : -1;
        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.push_back(state);
        previousInputStates_.push_back(state);

        std::vector<Model> models = std::vector<Model>();
        if (m_bServer)
        {
            PhysicalInfo pi = PhysicalInfo();
            pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
            pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
            pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
            pi.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
            pi.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
            Model model(ASSETS_DIR "/models/tie2/bland-tie.obj", pi, *assMan_, ModelType::PLAYER);
            models.push_back(model);
        }

        SetupScene(models);

        networking_ = new Networking(true, *scene_);
    }
    else
    {
        playerId_ = 1;
        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.push_back(state);
        previousInputStates_.push_back(state);

        auto models = BasicLevel();
        SetupScene(models);
    }
}

AssetManager& Engine::GetAssMan()
{
    return *assMan_;
}

std::vector<Model> Engine::BasicLevel()
{
    std::vector<Model> models = std::vector<Model>();

    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
    pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    pi.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);

    Model model(ASSETS_DIR "/models/tie2/bland-tie.obj", pi, *assMan_, ModelType::PLAYER);
    models.push_back(model);

    PhysicalInfo pi2 = PhysicalInfo();
    pi2.position_ = glm::vec3(-20.0f, 0.0f, 0.0f);
    pi2.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi2.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi2.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    pi2.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);

    Model model2(ASSETS_DIR "/models/tie2/bland-tie.obj", pi2, *assMan_, ModelType::PLAYER);
    models.push_back(model2);

    return models;
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
        ReconcileNetwork();
        CollectInputs(deltaTime);
        ExecuteInput(deltaTime);
        HandleLogic(deltaTime);
        if (m_bNetworking && m_bServer)
            networking_->SendGameState(*scene_, deltaTime);

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height, settings_);
        window_->SwapBuffers();

        i++;
        if (accTime >= 1) {
            std::cout << "Current FPS: " << i / accTime << std::endl;
            i = 0;
            accTime = 0;
        }
    }
    
    if (networking_ != nullptr)
        networking_->Shutdown();

    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    delete window_.get();
}

void Engine::ReconcileNetwork()
{
    if (m_bNetworking && !m_bServer)
    {

    }
}

void Engine::HandleLogic(float deltaTime)
{
    MoveModels();
    
    bool adjust = settings_["adjust_camera"];

    if (adjust)
        AdjustCamera();
    
    if (!m_bNetworking || m_bNetworking && m_bServer)
        CheckHits();
}

void Engine::CheckHits()
{
    auto playerModels = scene_->GetPlayerModels();
    
    // ToDo: change to auto &other
    for (auto other : scene_->GetModels())
    {
        glm::vec3 otherPos = other.model.GetPosition();
        float otherRadius = other.model.radius;

        for (auto& playerRef : playerModels)
        {
            auto& player = playerRef.get();

            if (player.Id == other.Id)
                continue;
                
            glm::vec3 playerPos = player.model.GetPosition();
            float playerRadius = player.model.radius;

            float dist = glm::length(otherPos - playerPos);
            float radiusSum = otherRadius + playerRadius;

            if (dist <= radiusSum)
            {
                glfwSetWindowShouldClose(window_->Get(), true);
            }
        }
    }
}

void Engine::AdjustCamera()
{
    if (settings_["third_person"] ) {
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
    pi.position_ = shooter.GetPosition() + shooter.GetOrientation();
    pi.rotation_ = shooter.GetRotation();
    pi.scale_ = glm::vec3(0.05f, 0.05f, 0.05f);
    pi.orientation_ = shooter.GetOrientation();
    pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi.speed_ = shooter.GetSpeed() + glm::vec3(0.1f, 0.0f, 0.0f);

    Model shot(ASSETS_DIR "/models/shot/longshot.obj", pi, *assMan_, ModelType::OBJECT, true, 0.05f);

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
    for (auto it : scene_->GetModels())
    {
        glm::vec3 change = it.model.GetOrientation() * it.model.GetSpeed().x;
        MoveModel(it.Id, change);

        auto position = it.model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && it.model.type_ != ModelType::PLAYER)
            deletes.push_back(it.Id);
    }

    for (auto id : deletes) 
        scene_->RemoveModel(id);
}

void Engine::RotateModel(unsigned int id, glm::vec3 change) 
{
    Model& model = scene_->GetModelByReference(id);
    glm::vec3 rotation = model.GetRotation();
    rotation += change;
    model.SetRotation(rotation);
}

void Engine::ChangeSetting(std::string key, bool value)
{
    settings_[key] = value;
}

void Engine::CollectInputs(float deltaTime)
{
    previousInputStates_[0] = currentInputStates_[0];

    currentInputStates_[0].left      = glfwGetKey(window_->Get(), GLFW_KEY_A) == GLFW_PRESS;
    currentInputStates_[0].right     = glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS;
    currentInputStates_[0].forward   = glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS;
    currentInputStates_[0].backward  = glfwGetKey(window_->Get(), GLFW_KEY_S) == GLFW_PRESS;
    currentInputStates_[0].shoot     = glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS;
    currentInputStates_[0].shootShot = false;

    bool shootPressed  =  currentInputStates_[0].shoot && !previousInputStates_[0].shoot;
    bool shootReleased = !currentInputStates_[0].shoot &&  previousInputStates_[0].shoot;

    if (lastShot > 0)
        lastShot -= deltaTime;

    if (lastShot < 0)
        lastShot = 0;

    if (shootPressed && lastShot == 0) {
        currentInputStates_[0].shootShot = true;
        lastShot = 5;
    }

    if (m_bNetworking)
    {
        // To Network
        networking_->SendInputState(currentInputStates_[0]);
    }
}

void Engine::ExecuteInput(float deltaTime)
{
    if (playerId_ < 1) return;

    for (auto& state : currentInputStates_)
    {
        if (state.left) 
            RotateModel(playerId_, {0.0f, glm::radians(0.7f), 0.0f});

        if (state.right)
            RotateModel(playerId_, {0.0f, glm::radians(-0.7f), 0.0f});

        if (state.forward) 
        {
            float acc = deltaTime * 0.05f;
            glm::vec3 speed = scene_->GetModelByReference(playerId_).GetSpeed();
            speed.x += acc;

            // Max Speed
            if (0.1f > speed.x)
                scene_->GetModelByReference(playerId_).SetSpeed(speed);
        } else 
        {
            float acc = deltaTime * 0.05f;
            if (state.backward)
                acc = deltaTime * .2f;
            glm::vec3 speed = scene_->GetModelByReference(playerId_).GetSpeed();

            if (speed.x > 0) speed.x -= acc;
            if (speed.x < 0) speed.x = 0;

            scene_->GetModelByReference(playerId_).SetSpeed(speed);
        }

        if (state.shootShot)
            Shoot(scene_->GetModelByReference(playerId_));
    }
    
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine) 
    {
        // Hitboxes
        if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
            engine->renderer_->ToggleHitboxes();
        // Skybox
        if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
            engine->renderer_->ToggleSkyBox();
        // Camera
        if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
            engine->settings_["adjust_camera"] = !engine->settings_["adjust_camera"];
        // Camara
        if (key == GLFW_KEY_F11 && action == GLFW_PRESS) 
        {
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
        // Bloom
        if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
            engine->settings_["bloom"] = !engine->settings_["bloom"];
        // Quit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
}
