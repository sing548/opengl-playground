#include "engine.h"

#include "../game/spawner/spawner.h"
#include "../game/systems/system-structs.h"
#include "../game/networking/network-bridge/network-bridge.h"

Engine::Engine(EngineMode config, const std::string& serverAddr, int port)
{
    if (config == EngineMode::Server)
    {
        m_bNetworking = true;
        m_bServer = true;
    }
    else if (config == EngineMode::Client)
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
    bool simpleFlight = true;
    settings_["simple_flight"] = simpleFlight;
    bool predictiveClient = false;
    settings_["predictive_client"] = predictiveClient;

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, -90.0F);
    
    inputManager_ = std::make_unique<InputManager>();
    window_ = std::make_unique<Window>(WIDTH, HEIGHT, std::move(camera_), inputManager_.get());
    renderer_ = std::make_unique<Renderer>(WIDTH, HEIGHT);
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
        playerId_ = 0;
       
        if (m_bServer)
        {
            PhysicalInfo pi = PhysicalInfo();
            pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
            pi.rotation_ = glm::quat(1, 0, 0, 0);
            pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

            playerId_ = spawner::SpawnPlayer(gameWorld_, *assMan_, pi);

            auto state = InputState { playerId_, false, false, false, false, false, false  };
            currentInputStates_.try_emplace(playerId_, state);
            previousInputStates_.try_emplace(playerId_, state);

        }
    
        if (m_bServer)
            netwBridg_ = std::make_unique<NetworkBridge>(NetworkBridge::Role::Server, serverAddr, port);
        else
            netwBridg_ = std::make_unique<NetworkBridge>(NetworkBridge::Role::Client, serverAddr, port);
    }
    else
    {
        BasicLevel();

        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.try_emplace(playerId_, state);
        previousInputStates_.try_emplace(playerId_, state);
    }

    TempBuildRenderHelpers();
}

Engine::~Engine() = default;

void Engine::TempBuildRenderHelpers()
{
    std::string modelVert  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.vert").string();
    std::string modelFrag  = (std::filesystem::path(FileHelper::GetShaderDir()) / "model.frag").string();
	std::string hitboxVert = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.vert").string();
	std::string hitboxFrag = (std::filesystem::path(FileHelper::GetShaderDir()) / "hitbox.frag").string();
    
    modelShader_  = std::make_unique<Shader>(modelVert.c_str(), modelFrag.c_str());
	hitboxShader_ = std::make_unique<Shader>(hitboxVert.c_str(), hitboxFrag.c_str());

    modelMat_ = std::make_unique<ModelMaterial>(modelShader_.get());
	hitboxMat_ = std::make_unique<HitboxMaterial>(hitboxShader_.get());
}

AssetManager& Engine::GetAssMan()
{
    return *assMan_;
}

void Engine::BasicLevel()
{
    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
    pi.rotation_ = glm::quat(1, 0, 0, 0);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

    playerId_ = spawner::SpawnPlayer(gameWorld_, *assMan_, pi);

    PhysicalInfo pi2 = PhysicalInfo();
    pi2.position_ = glm::vec3(-20.0f, 0.0f, 0.0f);
    pi2.rotation_ = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
    pi2.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

    spawner::SpawnNpc(gameWorld_, *assMan_, pi2);
}

void Engine::Run()
{
    // FixedDelta = Logic / s
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
        deltaTime = std::min(deltaTime, 0.25f);
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

            if (m_bNetworking && m_bServer)
            {
               netwBridg_->ManageGameStateDistribution(gameWorld_, fixedDelta);
            }
            else if (m_bNetworking && playerId_ > 0)
            {
                netwBridg_->SendInputState(currentInputStates_.at(playerId_));
            }

            j++;
            // Logic/s counter in console
            if (logicTime >= 1) 
            {
                std::cout << "Current Logic/s: " << j / logicTime << std::endl;
                j = 0;
                logicTime = 0;
            }

            gameWorld_.GetScene().ClearAddedModels();
        }

        if (m_bNetworking && !m_bServer)
        {
            bool predictive = settings_["predictive_client"];
            netwBridg_->MergeClientWithNetwork(gameWorld_, *assMan_, predictive);

            if (predictive)
            {
                auto& statesToReplay = netwBridg_->ResetPlayerToLastInputState(gameWorld_);
    
                SystemsContext context = SystemsContext
                {
                    deltaTime,
                    gameWorld_,
                    *assMan_,
                    *netwBridg_,
                    currentInputStates_,
                    previousInputStates_,
                    playerId_,
                    !(m_bNetworking && !m_bServer),
                    true,
                    settings_
                };
    
                for (auto& [tick, state] : statesToReplay)
                {
                    stateAsMap_[playerId_] = state;
                    // Not saving "previuousStateAsMap" - would need updating if 
                    playerSystem_.Update(context);
                    physicsSystem_.Update(context);
                }
            }
        }

        bool adjust = settings_["adjust_camera"];
        if (adjust)
            AdjustCamera();

        auto [rL, fG] = BuildRenderList();
        
        renderer_->Draw(rL, fG, settings_);
        
        window_->SwapBuffers();

        i++;
        // FPS counter in console
        if (fpsTime >= 1) {
            std::cout << "Current FPS: " << i / fpsTime << std::endl;
            i = 0;
            fpsTime = 0;
        }
    }
    
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
}

void Engine::ReconcileNetwork()
{
    if (m_bNetworking) killedPlayers_.clear();
    
    if (m_bNetworking && !m_bServer)
    {
        UpdateClientNetworking();
    }
    else if (m_bNetworking)
    {
        UpdateServerNetworking();
    }
}

void Engine::UpdateServerNetworking()
{
    netwBridg_->PollEvents(gameWorld_, *assMan_);
    auto inputStates = netwBridg_->GetInputStates();

    for (const auto [id, state] : inputStates)
    {
        if (std::ranges::find(killedPlayers_, id) != killedPlayers_.end()) 
            continue;

        auto it = currentInputStates_.find(state.id);
        
        if (it == currentInputStates_.end())
        {
            previousInputStates_[state.id] = state;
        }
        else if (previousInputStates_.at(state.id).tick - 1 < netwBridg_->GetCurrentTick())
        {
            previousInputStates_.at(state.id) = currentInputStates_.at(state.id);
        }

        currentInputStates_[state.id] = state;
    }
}

void Engine::UpdateClientNetworking()
{
    netwBridg_->PollEvents(gameWorld_, *assMan_);
    auto playerId = netwBridg_->GetPlayerId();

    if (playerId != 0)
    {
        playerId_ = playerId;
        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.try_emplace(playerId_, state);
        previousInputStates_.try_emplace(playerId_, state);
    }
}

void Engine::HandleLogic(float deltaTime)
{
    auto removed = gameWorld_.RemoveMarkedEntities();
    
    for (auto& id : removed.all)
    {
        currentInputStates_.erase(id);
        previousInputStates_.erase(id);
        if (id == playerId_) playerId_ = 0;
    }

    for (uint32_t id : removed.players)
    {
        killedPlayers_.push_back(id);
    }

    SystemsContext context = SystemsContext
    {
        deltaTime,
        gameWorld_,
        *assMan_,
        *netwBridg_,
        currentInputStates_,
        previousInputStates_,
        playerId_,
        !(m_bNetworking && !m_bServer),
        false,
        settings_
    };
    
    physicsSystem_.Update(context);
    npcSystem_.Update(context);
    playerSystem_.Update(context);
    shotSystem_.Update(context);
}

void Engine::AdjustCamera()
{
    if (settings_["third_person"]) {
        if (playerId_ == 0 || !gameWorld_.GetScene().ModelExists(playerId_)) return;

        const auto& model = gameWorld_.GetScene().GetModelByReference(playerId_);
        glm::vec3 modelPos = model.GetPosition();
        
        // Offset from the model in its local orientation
        glm::vec3 offset = model.GetForward() * glm::vec3(-8.0f, -8.0f, -8.0f);
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
        float requiredX = (abs(gameWorld_.GetScene().currentFurthestPosition.x) + 1.5f) / (halfFovTan * aspect);
        float requiredZ = (abs(gameWorld_.GetScene().currentFurthestPosition.z) + 1.5f)/ halfFovTan;

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
    pi.rotation_ = glm::quat(1, 0, 0, 0);
    pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

    spawner::SpawnPlayer(gameWorld_, *assMan_, pi, id);
    
    const auto& playerData = gameWorld_.GetPlayerData();

    for (auto& [pId, _] : playerData)
    {
        const std::vector<unsigned int> addedModels = gameWorld_.GetScene().GetAddedModels();

        if (std::find(addedModels.begin(), addedModels.end(), pId) == addedModels.end())
            gameWorld_.GetScene().AddExtraModelToAddedIds(pId);
    }
}

void Engine::ChangeSetting(std::string key, bool value)
{
    settings_[key] = value;
}

void Engine::CollectInputs(float deltaTime)
{
    if (playerId_ == 0 || std::ranges::find(killedPlayers_, playerId_) != killedPlayers_.end()) return;
    if (!currentInputStates_.contains(playerId_)) return;

    previousInputStates_.at(playerId_) = currentInputStates_.at(playerId_);
    
    InputState state = currentInputStates_.at(playerId_);

    state.left      = glfwGetKey(window_->Get(), GLFW_KEY_A) == GLFW_PRESS;
    state.right     = glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS;
    state.forward   = glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS;
    state.backward  = glfwGetKey(window_->Get(), GLFW_KEY_S) == GLFW_PRESS;
    state.shoot     = glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS;

    currentInputStates_.at(playerId_) = state;
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine) 
    {
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            engine->settings_["predictive_client"] = !engine->settings_["predictive_client"];
        if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
        {
            auto it = engine->currentInputStates_.find(engine->playerId_);
            if (it == engine->currentInputStates_.end())
            {
                PhysicalInfo pi = PhysicalInfo();
                pi.position_ = glm::vec3(20.0f, 0.0f, 0.0f);
                pi.rotation_ = glm::quat(1, 0, 0, 0);
                pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);

                engine->playerId_ = spawner::SpawnPlayer(engine->gameWorld_, *engine->assMan_, pi);

                auto state = InputState { engine->playerId_, false, false, false, false, false, false  };
                engine->currentInputStates_.try_emplace(engine->playerId_, state);
                engine->previousInputStates_.try_emplace(engine->playerId_, state);
            }
        }
        if (key == GLFW_KEY_F6 && action == GLFW_PRESS && engine->m_bNetworking && engine->m_bServer)
        {
            engine->netwBridg_->RespawnPlayers(engine->gameWorld_, *engine->assMan_);
        }
        // Simple Flight
        if (key == GLFW_KEY_F7 && action == GLFW_PRESS)
            engine->settings_["simple_flight"] = !engine->settings_["simple_flight"];
        // Hitboxes
        if (key == GLFW_KEY_F8 && action == GLFW_PRESS)
            engine->settings_["hitboxes"] = !engine->settings_["hitboxes"];
            //engine->renderer_->ToggleHitboxes();
        // Skybox
        if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
            engine->settings_["sky_box"] = !engine->settings_["sky_box"];
            //engine->renderer_->ToggleSkyBox();
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

std::tuple<RenderList, FrameGlobals> Engine::BuildRenderList()
{
    glm::mat4 projection = glm::perspective(glm::radians(window_->GetCamera().Zoom), (float)window_->GetSize().width / (float)window_->GetSize().height, 0.1f, 500.0f);
	glm::mat4 view = window_->GetCamera().GetViewMatrix();
	auto& models = gameWorld_.GetScene().GetModels();

	FrameGlobals fg;
	
	fg.view 	  = view;
	fg.projection = projection;
	fg.cameraPos  = window_->GetCamera().Position;
	fg.time 	  = 0.0f;

	for (auto& [id, model] : models)
	{
		if (gameWorld_.IsShot(id)) {
			PointLight pl;
			pl.position = model.GetPosition();
			fg.pointLights.push_back(pl);
		}
	}

	RenderList rl;

	for (auto& [id, model] : models)
	{
		glm::mat4 modelProjection = glm::mat4(1.0f);
		modelProjection = glm::translate(modelProjection, model.GetPosition());
		modelProjection *=  glm::mat4_cast(glm::quat(model.GetRotation()));
		modelProjection = glm::scale(modelProjection, model.GetScale());

		for (auto& mesh : model.GetMeshes())
		{
			DrawCommand dc;

			dc.mesh = mesh.get();
			dc.material = modelMat_.get();
			dc.transform = modelProjection;
			dc.tint = {1,1,1,1};
			dc.renderPass = RenderPass::Opaque;

			rl.commands.push_back(dc);
		}

		if (settings_["hitboxes"])
		{
			DrawCommand dc;

			dc.mesh = model.GetHitboxMesh();
			dc.material = hitboxMat_.get();
			
			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetPosition());
    		modelMat = glm::scale(modelMat, glm::vec3(model.GetRadius()));

			dc.transform = modelMat;
			dc.tint = {1,1,1,1};
			dc.renderPass = RenderPass::Debug;

			rl.commands.push_back(dc);
		}
	}

	for (auto& [id, playerData] : gameWorld_.GetPlayerData())
	{
		if (playerData.lastHit > 0)
		{
			auto& model = gameWorld_.GetScene().GetModelByReference(id);
			
			DrawCommand dc;
			dc.mesh = model.GetHitboxMesh();
			dc.material = hitboxMat_.get();

			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetPosition());
    		modelMat = glm::scale(modelMat, glm::vec3(model.GetRadius()));

			dc.transform = modelMat;
			dc.tint = {1,1,1,1};
			dc.renderPass = RenderPass::Opaque;

			rl.commands.push_back(dc);
		}
	}

    for (auto& [id, npcData] : gameWorld_.GetNpcData())
    {
        if (npcData.lastHit > 0)
		{
			auto& model = gameWorld_.GetScene().GetModelByReference(id);
			
			DrawCommand dc;
			dc.mesh = model.GetHitboxMesh();
			dc.material = hitboxMat_.get();

			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetPosition());
    		modelMat = glm::scale(modelMat, glm::vec3(model.GetRadius()));

			dc.transform = modelMat;
			dc.tint = {1,1,1,1};
			dc.renderPass = RenderPass::Opaque;

			rl.commands.push_back(dc);
		}
    }

	modelMat_->ApplyFrame(fg);

	Material* lastMat = nullptr;

	for (auto& dc : rl.commands)
	{
		if (dc.material != lastMat)
		{ 
			dc.material->ApplyFrame(fg);
			lastMat = dc.material;
		}

		dc.material->ApplyInstance(dc.transform, dc.tint);

		dc.mesh->Draw(dc.material->GetShader());
	}

    return std::tuple(rl, fg);
};
