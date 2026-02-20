#include "engine.h"

Engine::Engine(int config, const char *serverAddr)
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

        PlayerData pd;
        pd.id = 1;
        pd.lastHit = 0;
        scene_->AddOrUpdatePlayerData(pd);
        
        if (m_bServer)
            networking_ = new Networking(true, *scene_);
        else
        {
            networking_ = new Networking(false, *scene_, serverAddr);
        }
    }
    else
    {
        playerId_ = 1;
        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.try_emplace(playerId_, state);
        previousInputStates_.try_emplace(playerId_, state);

        auto models = BasicLevel();
        SetupScene (models);

        PlayerData pd;
        pd.id = 1;
        scene_->AddOrUpdatePlayerData(pd);

        PlayerData pd2;
        pd2.id = 2;
        scene_->AddOrUpdatePlayerData(pd2);
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
    const float fixedDelta = 1.0f / 60.0f;

    int i = 0, j = 0;
    float accTime = 0.0f;
    float fpsTime = 0.0f;
    float logicTime = 0.0f;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    uint32_t newClient = 0;

    while (!glfwWindowShouldClose(window_->Get())) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
	    lastFrame = currentFrame;
        accTime += deltaTime;
        fpsTime += deltaTime;
        logicTime += deltaTime;
        
        glfwPollEvents();

        while (accTime >= fixedDelta)
        {
            ReconcileNetwork();
            CollectInputs(fixedDelta);
            HandleLogic(fixedDelta);

            accTime -= fixedDelta;

            if (newClient != 0)
            {
                AddNewPlayer(newClient);
            }
    
            newClient = 0;
    
            if (m_bNetworking && m_bServer)
            {
               newClient = networking_->SendGameState(*scene_, scene_->GetAddedModels(), scene_->GetRemoveMarkedModels(), fixedDelta);    
            }
            else if (m_bNetworking && playerId_ >= 0)
            {
                networking_->SendInputState(currentInputStates_.at(playerId_));
            }

            scene_->RemoveMarkedModels();
            scene_->ClearAddedModels();

            j++;
            // Logic/s counter in console
            if (logicTime >= 1) 
            {
                std::cout << "Current Logic/s: " << j / logicTime << std::endl;
                j = 0;
                logicTime = 0;
            }
        }

        renderer_->Draw(*scene_, window_->GetCamera(), window_->GetSize().width, window_->GetSize().height, settings_);
        window_->SwapBuffers();

        i++;
        // FPS counter in console
        if (fpsTime >= 1) {
            std::cout << "Current FPS: " << i / fpsTime << std::endl;
            i = 0;
            fpsTime = 0;
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
        auto [playerId, playerModelRemoved] = networking_->UpdateScene(*scene_, *assMan_);
        if (playerId != 0)
        {
            playerId_ = playerId;
            auto state = InputState { playerId_, false, false, false, false, false, false  };
            currentInputStates_.try_emplace(playerId_, state);
            previousInputStates_.try_emplace(playerId_, state);
        }

        for (auto& id : playerModelRemoved)
        {
            auto i = currentInputStates_.find(id);
            auto j = previousInputStates_.find(id);

            if (i != currentInputStates_.end())
                currentInputStates_.erase(i);
            if (j != previousInputStates_.end())
                previousInputStates_.erase(j);
            
            if (id == playerId_)
                playerId_ = -1;
            
            scene_->RemovePlayerData(id);
        }
    }
    else if (m_bNetworking)
    {
        auto inputStates = networking_->GetInputStates();

        for (const auto [id, state] : inputStates)
        {
            if (std::ranges::find(killedPlayers_, id) != killedPlayers_.end()) 
                continue;

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
    physicsSystem_.Update(deltaTime, *scene_, !m_bNetworking || m_bNetworking && m_bServer, *window_, (m_bNetworking && m_bServer) || !m_bNetworking);
    auto removes = scene_->GetRemoveMarkedModels();
    
    for (auto& r : removes)
    {
        auto model = scene_->GetModelByReference(r);
        if (model.type_ == ModelType::PLAYER)
        {
            auto i = currentInputStates_.find(r);
            auto j = previousInputStates_.find(r);

            currentInputStates_.erase(i);
            previousInputStates_.erase(j);

            killedPlayers_.push_back(r);
        }
    }

    playerSystem_.Update(deltaTime, *scene_, *assMan_, currentInputStates_, previousInputStates_, playerId_, !(m_bNetworking && !m_bServer));
    shotSystem_.Update(*scene_);
    cameraSystem_.Update();
    
    bool adjust = settings_["adjust_camera"];

    if (adjust)
        AdjustCamera();
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
        float requiredX = (abs(scene_->currentFurthestPosition_.x) + 1.5f) / (halfFovTan * aspect);
        float requiredZ = (abs(scene_->currentFurthestPosition_.z) + 1.5f)/ halfFovTan;

        // Take the larger
        float h = std::max(requiredX, requiredZ);

        // Clamp
        h = std::clamp(h, minHeight, maxHeight);

        float oldHeight = window_->GetCamera().Position.y;

        float changeRate = 0.5f;

        if (abs(h - oldHeight) > changeRate) {
            if (h > oldHeight) h = oldHeight + changeRate;
            if (h < oldHeight) h = oldHeight - changeRate;
        }

        window_->UpdateCameraPosition(glm::vec3(0, h, 0));
    }
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

    scene_->AddModel(model, id);

    PlayerData pd;
    pd.id = id;

    scene_->AddOrUpdatePlayerData(pd);
    
    auto playerModels = scene_->GetPlayerModels();

    for (auto& [id, model] : playerModels)
    {
        const std::vector<unsigned int> addedModels = scene_->GetAddedModels();

        if (std::find(addedModels.begin(), addedModels.end(), id) == addedModels.end())
            scene_->AddExtraModelToAddedIds(id);
    }
}

void Engine::ChangeSetting(std::string key, bool value)
{
    settings_[key] = value;
}

void Engine::CollectInputs(float deltaTime)
{
    if (playerId_ < 0 || std::ranges::find(killedPlayers_, playerId_) != killedPlayers_.end()) return;
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
