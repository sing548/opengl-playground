#include "engine.h"

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
       

        std::vector<Model> models = std::vector<Model>();
        if (m_bServer)
        {
            auto state = InputState { playerId_, false, false, false, false, false, false  };
            currentInputStates_.try_emplace(playerId_, state);
            previousInputStates_.try_emplace(playerId_, state);

            PhysicalInfo pi = PhysicalInfo();
            pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
            pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
            pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
            pi.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
            pi.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
            Model model(Model::GetModelPath(ModelType::PLAYER), pi, *assMan_, ModelType::PLAYER);
            models.push_back(model);
        }
        
        SetupScene(models);
        
        if (m_bServer)
            networking_ = new Networking(true, *scene_);
        else
            networking_ = new Networking(false, *scene_);
    }
    else
    {
        playerId_ = 1;
        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.try_emplace(playerId_, state);
        previousInputStates_.try_emplace(playerId_, state);

        auto models = BasicLevel();
        SetupScene (models);
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

    Model model(Model::GetModelPath(ModelType::PLAYER), pi, *assMan_, ModelType::PLAYER);
    models.push_back(model);

    PhysicalInfo pi2 = PhysicalInfo();
    pi2.position_ = glm::vec3(-20.0f, 0.0f, 0.0f);
    pi2.rotation_ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
    pi2.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi2.orientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi2.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);

    Model model2(Model::GetModelPath(ModelType::PLAYER), pi2, *assMan_, ModelType::PLAYER);
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
    uint32_t newClient = 0;

    while (!glfwWindowShouldClose(window_->Get())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;
         
        gameClock_ = std::chrono::steady_clock::now();

        removedModels.clear();
        addedModels.clear();
        
        glfwPollEvents();
        ReconcileNetwork();
        CollectInputs(deltaTime);
        ExecuteInput(deltaTime);
        HandleLogic(deltaTime);

        if (newClient != 0)
        {
            AddNewPlayer(newClient);
        }

        newClient = 0;

        if (m_bNetworking && m_bServer)
        {
           newClient = networking_->SendGameState(*scene_, addedModels, removedModels, deltaTime);    
        }
        else if (m_bNetworking && playerId_ >= 0)
        {
            networking_->SendInputState(currentInputStates_.at(playerId_));
        }

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height, settings_);
        window_->SwapBuffers();

        i++;
        // FPS counter in console
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
        auto playerId = networking_->UpdateScene(*scene_, *assMan_);
        if (playerId != 0)
        {
            playerId_ = playerId;
            auto state = InputState { playerId_, false, false, false, false, false, false  };
            currentInputStates_.try_emplace(playerId_, state);
            previousInputStates_.try_emplace(playerId_, state);
        }
    }
    else if (m_bNetworking)
    {
        auto inputStates = networking_->GetInputStates();

        for (const auto [id, state] : inputStates)
        {
            auto it = currentInputStates_.find(state.id);
            
            if (it == currentInputStates_.end())
            {
                previousInputStates_[state.id] = state;
            }
            else if (previousInputStates_.at(state.id).tick - 1 < networking_->currentTick)
            {
                previousInputStates_.at(state.id) = currentInputStates_.at(state.id);
            }

            currentInputStates_[state.id] = state;
        }
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

    for (auto& [id, other] : scene_->GetModels())
    {
        glm::vec3 otherPos = other.GetPosition();
        float otherRadius = other.GetRadius();

        for (auto& [playerId, playerRef] : playerModels)
        {
            auto& player = playerRef.get();

            if (playerId == id)
                continue;
                
            glm::vec3 playerPos = player.GetPosition();
            float playerRadius = player.GetRadius();

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
        auto model = scene_->GetModelByReference(playerId_);
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
    if (m_bNetworking && !m_bServer) return;

    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = shooter.GetPosition() + shooter.GetOrientation();
    pi.rotation_ = shooter.GetRotation();
    pi.scale_ = glm::vec3(0.05f, 0.05f, 0.05f);
    pi.orientation_ = shooter.GetOrientation();
    pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi.speed_ = shooter.GetSpeed() + glm::vec3(0.1f, 0.0f, 0.0f);

    Model shot(Model::GetModelPath(ModelType::SHOT), pi, *assMan_, ModelType::SHOT, true, 0.05f);

    AddModel(shot);
}

void Engine::AddNewPlayer(uint32_t id)
{
    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
    pi.rotation_ = glm::vec3(0.0f, 0.0f, 0.0f);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
    pi.orientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    pi.baseOrientation_ = glm::vec3(-1.0f, 0.0f, 0.0f);
    Model model(Model::GetModelPath(ModelType::PLAYER), pi, *assMan_, ModelType::PLAYER);

    AddModel(model, id);
    
    auto playerModels = scene_->GetPlayerModels();

    for (auto& [id, model] : playerModels)
    {
        if (std::find(addedModels.begin(), addedModels.end(), id) == addedModels.end())
            addedModels.push_back(id);
    }
}

void Engine::AddModel(Model& model, uint32_t id)
{
    unsigned int modelId;

    if (id == 0)
    {
        modelId = scene_->AddModel(model);
    }
    else
    {
        modelId = scene_->AddModelWithId(model, id);
    }
    
    addedModels.push_back(modelId);
}

void Engine::RemoveModel(unsigned int id)
{
    scene_->RemoveModel(id);
    removedModels.push_back(id);
}

void Engine::MoveModel(unsigned int id, const glm::vec3& change)
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
    for (auto& [id, model] : scene_->GetModels())
    {
        glm::vec3 change = model.GetOrientation() * model.GetSpeed().x;
        MoveModel(id, change);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && model.type_ != ModelType::PLAYER)
            deletes.push_back(id);
    }

    for (auto id : deletes)
        RemoveModel(id);
}

void Engine::RotateModel(unsigned int id, const glm::vec3& change) 
{
    Model& model = scene_->GetModelByReference(id);
    model.SetRotation(model.GetRotation() + change);
}

void Engine::ChangeSetting(std::string key, bool value)
{
    settings_[key] = value;
}

void Engine::CollectInputs(float deltaTime)
{
    if (playerId_ < 0) return;
    auto state = currentInputStates_.at(playerId_);
    auto previousState = previousInputStates_.at(playerId_);
    previousState = currentInputStates_.at(playerId_);

    state.left      = glfwGetKey(window_->Get(), GLFW_KEY_A) == GLFW_PRESS;
    state.right     = glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS;
    state.forward   = glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS;
    state.backward  = glfwGetKey(window_->Get(), GLFW_KEY_S) == GLFW_PRESS;
    state.shoot     = glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS;
    state.shootShot = false;

    bool shootPressed  =  state.shoot && !previousState.shoot;
    bool shootReleased = !state.shoot &&  previousState.shoot;

    if (lastShot > 0)
    {
        lastShot -= deltaTime > lastShot ? lastShot : deltaTime;
    }
    
    if (shootPressed && lastShot == 0) {
        state.shootShot = true;
        lastShot = 5;              
    }

    currentInputStates_.at(playerId_) = state;
    previousInputStates_.at(playerId_) = previousState;
}

void Engine::ExecuteInput(float deltaTime)
{
    bool playerExists = scene_->ModelExists(playerId_);
    if (playerId_ <= 0 || !playerExists) return;

    for (auto& [playerId, state] : currentInputStates_)
    {
        if (state.left) 
            RotateModel(playerId, {0.0f, glm::radians(0.7f), 0.0f});

        if (state.right)
            RotateModel(playerId, {0.0f, glm::radians(-0.7f), 0.0f});

        if (state.forward) 
        {
            float acc = deltaTime * 0.05f;
            glm::vec3 speed = scene_->GetModelByReference(playerId).GetSpeed();
            speed.x += acc;

            // Max Speed
            if (0.1f > speed.x)
                scene_->GetModelByReference(playerId).SetSpeed(speed);
        } else 
        {
            float acc = deltaTime * 0.05f;
            if (state.backward)
                acc = deltaTime * .2f;
            glm::vec3 speed = scene_->GetModelByReference(playerId).GetSpeed();

            if (speed.x > 0) speed.x -= acc;
            if (speed.x < 0) speed.x = 0;

            scene_->GetModelByReference(playerId).SetSpeed(speed);
        }

        if (state.shootShot && playerId == playerId_)
        {
            Shoot(scene_->GetModelByReference(playerId));
        }
        else 
        {
            if (state.shootShot && previousInputStates_.at(playerId).shootShot != true)
            {
                previousInputStates_.at(playerId).shootShot = true;
                Shoot(scene_->GetModelByReference(playerId));
            }
        }
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
