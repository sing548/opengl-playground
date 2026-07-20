#include "engine.h"

#include <cfloat>
#include <thread>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "logging/ext-std-log.h"
#include "systems/system-structs.h"
#include "net-transport/client-transport.h"
#include "net-transport/server-transport.h"

#include "../game/spawner/spawner.h"
#include "../game/networking/network-bridge/network-bridge.h"

Engine::Engine(EngineMode config, const std::string& serverAddr, int port)
{
    if (config == EngineMode::Server)
    {
        m_bNetworking = true;
        m_bServer = true;
        settings_.npcs = false;
    }
    else if (config == EngineMode::Client)
    {
        m_bNetworking = true;
    }

    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0F, -90.0F);
    
    inputManager_ = std::make_unique<InputManager>();
    window_ = std::make_unique<Window>(WIDTH, HEIGHT, std::move(camera_), inputManager_.get());
    renderer_ = std::make_unique<Renderer>(WIDTH, HEIGHT);
    assMan_ = std::make_unique<AssetManager>();
    audio_ = std::make_unique<AudioManager>();

    glfwSetWindowUserPointer(window_->Get(), this);
    glfwSetKeyCallback(window_->Get(), Engine::KeyCallback);

    glfwSetCursorPosCallback(window_->Get(), [](GLFWwindow* window, double xPos, double yPos) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

        if (engine && engine->settings_.mouseLooking) {
            engine->window_->mouse_callback(xPos, yPos);
        }
    });

	glfwSetFramebufferSizeCallback(window_->Get(), [](GLFWwindow* w, int width, int height)
	{
		Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
		engine->GetWindow().ResizeWindow(width, height);
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
            netwBridg_ = std::make_unique<NetworkBridge>(NetworkBridge::Role::Server, serverAddr, port, debugStats_);
        else
            netwBridg_ = std::make_unique<NetworkBridge>(NetworkBridge::Role::Client, serverAddr, port, debugStats_);
    }
    else
    {
        BasicLevel();

        auto state = InputState { playerId_, false, false, false, false, false, false  };
        currentInputStates_.try_emplace(playerId_, state);
        previousInputStates_.try_emplace(playerId_, state);

        netwBridg_ = std::make_unique<NetworkBridge>(NetworkBridge::Role::Offline, "", 0, debugStats_);
    }

    HandleImGui(0);
}

Engine::~Engine() = default;

void Engine::AddSceneRenderable(std::unique_ptr<ISceneRenderable> r)
{
    sceneRenderables_.push_back(std::move(r));
    int size = sceneRenderables_.size();
    renderer_->AddSceneRenderable(sceneRenderables_.at(size - 1).get());
};

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
    double lastFrame = 0.0f;

    int stepsHist[3] = {0, 0, 0};

    uint32_t lastReplayedMaxTick_ = 0;
    int numDistanceForReset = 0;
    float distanceForPlayerReset = 0.0f;
    float biggestDistanceForReset = 0.0f;
    glm::vec3 oldPlayerPos;
    uint32_t newClient = 0;

    SortSystems();

    while (!glfwWindowShouldClose(window_->Get())) {

        const float step = fixedDelta / (1.0f + netwBridg_->GetTimeDilation());

        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        deltaTime = std::min(deltaTime, 0.25f);
	    lastFrame = currentFrame;
        accTime += deltaTime;
        fpsTime += deltaTime;
        logicTime += deltaTime;
        
        glfwPollEvents();

        HandleImGui(1);

        if (window_->WasWindowResized()) 
        {
            renderer_->ResizeWindow(window_->GetSize().width, window_->GetSize().height);
            window_->ClearResizedFlag();
        }

        int stepsThisFrame = 0;

        ExecuteSystems(GameplayPhase::FrameStart, fixedDelta);

        while (accTime >= step)
        {
            ExecuteSystems(GameplayPhase::Input, fixedDelta);
            CollectInputs(fixedDelta);
            ExecuteSystems(GameplayPhase::PreSimulation, fixedDelta);
            HandleLogic(fixedDelta);
            ExecuteSystems(GameplayPhase::Simulation, fixedDelta);
            accTime -= step;
            ExecuteSystems(GameplayPhase::PostSimulation, fixedDelta);

            j++;
            // Logic/s counter in console
            if (logicTime >= 1) 
            {
                std::cout << "Current Logic/s: " << j / logicTime << std::endl;

                j = 0;
                logicTime = 0;
            }

            gameWorld_.GetScene().ClearAddedModels();

            ++stepsThisFrame;
        }
        ++stepsHist[std::min(stepsThisFrame, 2)];

        for (const auto& d : gameWorld_.DrainDeathSounds())
            audio_->PlayOneShot("sounds/ship_explosion.wav");

        for (WeaponSound w : gameWorld_.DrainLocalWeaponSounds())
        {
            if (w == WeaponSound::Laser)
                audio_->PlayOneShotPitched("sounds/laser_shot.wav", 0.9f, 1.1f);
            else
                audio_->PlayOneShotPitched("sounds/missile.mp3", 0.9f, 1.1f);
        }

        if (playerId_ != 0 && gameWorld_.GetPlayerData().contains(playerId_))
        {
            float curr = gameWorld_.GetPlayerData(playerId_).lastHit;
            if (curr > lastHitPrev_ + 0.01f)
                audio_->PlayOneShotPitched("sounds/hit_self.wav", 0.55f, 1.45f);
            lastHitPrev_ = curr;
        }
        else
        {
            lastHitPrev_ = 0.0f;
        }

        ExecuteSystems(GameplayPhase::PostTick, deltaTime);

        if (m_bNetworking && !m_bServer)
        {
            bool predictive = settings_.predictiveClient;

            if (predictive)
            {
                auto& statesToReplay = netwBridg_->ResetPlayerToLastInputState(gameWorld_);

                uint32_t maxTick = statesToReplay.empty() ? lastReplayedMaxTick_
                                          : statesToReplay.rbegin()->first;
                bool newInputThisFrame = (maxTick != lastReplayedMaxTick_);
                lastReplayedMaxTick_ = maxTick;
    
                SystemsContext context = SystemsContext
                {
                    fixedDelta,
                    *window_,
                    gameWorld_,
                    *assMan_,
                    *netwBridg_,
                    *terrainHandler_,
                    currentStateAsMap_,
                    previousStateAsMap_,
                    playerId_,
                    !(m_bNetworking && !m_bServer),
                    true,
                    0.0f,
                    settings_
                };
    
                for (auto& [tick, state] : statesToReplay)
                {
                    previousStateAsMap_[playerId_] = currentStateAsMap_[playerId_];
                    currentStateAsMap_[playerId_] = state;

                    for (auto& system : systems_)
                    {
                        if (!system->CanReplay()) continue;

                        system->Update(context);
                    }
                }
            }
        }

        ExecuteSystems(GameplayPhase::PreRender, deltaTime, accTime / step);

        auto [rL, fG] = BuildRenderList();
        
        renderer_->Draw(rL, fG, settings_);

        HandleImGui(2);
        
        window_->SwapBuffers();

        i++;
        // FPS counter in console
        if (fpsTime >= 1) {
            debugStats_.Flush(fpsTime);
            std::cout << "Current FPS: " << i / fpsTime << std::endl;
            i = 0;
            fpsTime = 0;
        }
    }
    
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
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
        gameWorld_.GetKilledPlayers().push_back(id);
    }
}

void Engine::ExecuteSystems(GameplayPhase phase, float dT, float alpha)
{
    SystemsContext context = SystemsContext
    {
        dT,
        *window_,
        gameWorld_,
        *assMan_,
        *netwBridg_,
        *terrainHandler_,
        currentInputStates_,
        previousInputStates_,
        playerId_,
        !(m_bNetworking && !m_bServer),
        false,
        alpha,
        settings_
    };

    for (auto& system : systems_)
    {
        if (system->GetPhase() != phase) continue;

        system->Update(context);
    }
}

void Engine::SortSystems()
{
    std::sort(systems_.begin(), systems_.end(),
        [](const std::unique_ptr<IGameplaySystem>& a,
           const std::unique_ptr<IGameplaySystem>& b)
        {
            if (a->GetPhase() != b->GetPhase())
                return a->GetPhase() < b->GetPhase();
            return a->GetOrder() < b->GetOrder();
        });
}

void Engine::SortRenderables()
{
    std::sort(sceneRenderables_.begin(), sceneRenderables_.end(),
        [](const std::unique_ptr<ISceneRenderable>& a,
           const std::unique_ptr<ISceneRenderable>& b)
        {
            return a->GetOrder() < b->GetOrder();
        });
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

void Engine::CollectInputs(float deltaTime)
{
    if (playerId_ == 0 || std::ranges::find(gameWorld_.GetKilledPlayers(), playerId_) != gameWorld_.GetKilledPlayers().end()) return;
    if (!currentInputStates_.contains(playerId_)) return;

    previousInputStates_.at(playerId_) = currentInputStates_.at(playerId_);
    
    InputState state = currentInputStates_.at(playerId_);

    state.left      = glfwGetKey(window_->Get(), GLFW_KEY_A) == GLFW_PRESS;
    state.right     = glfwGetKey(window_->Get(), GLFW_KEY_D) == GLFW_PRESS;
    state.forward   = glfwGetKey(window_->Get(), GLFW_KEY_W) == GLFW_PRESS;
    state.backward  = glfwGetKey(window_->Get(), GLFW_KEY_S) == GLFW_PRESS;
    state.shoot     = glfwGetKey(window_->Get(), GLFW_KEY_SPACE) == GLFW_PRESS
                   || glfwGetMouseButton(window_->Get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    state.homingShoot = glfwGetMouseButton(window_->Get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    double xpos, ypos;
    glfwGetCursorPos(window_->Get(), &xpos, &ypos);

    auto windowSize = window_->GetSize();
    float winX = static_cast<float>(xpos);
    float winY = static_cast<float>(windowSize.height) - static_cast<float>(ypos);

    glm::mat4 projection = glm::perspective(
        glm::radians(window_->GetCamera().Zoom),
        static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height),
        0.1f, 500.0f);
    glm::mat4 view = window_->GetCamera().GetViewMatrix();
    glm::vec4 viewport(0.0f, 0.0f,
                       static_cast<float>(windowSize.width),
                       static_cast<float>(windowSize.height));

    glm::vec3 nearPt = glm::unProject(glm::vec3(winX, winY, 0.0f), view, projection, viewport);
    glm::vec3 farPt  = glm::unProject(glm::vec3(winX, winY, 1.0f), view, projection, viewport);
    glm::vec3 dir = farPt - nearPt;

    float planeY = 0.0f;
    if (gameWorld_.GetScene().ModelExists(playerId_))
        planeY = gameWorld_.GetScene().GetModelByReference(playerId_).GetPosition().y;

    if (std::abs(dir.y) > 1e-6f)
    {
        float t = (planeY - nearPt.y) / dir.y;
        if (t > 0.0f)
        {
            glm::vec3 hit = nearPt + dir * t;
            state.aim_x = hit.x;
            state.aim_z = hit.z;
        }
    }

    currentInputStates_.at(playerId_) = state;
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));

    if (engine) 
    {
        // Respawn self
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
        // Respawn clients
        if (key == GLFW_KEY_F6 && action == GLFW_PRESS && engine->m_bNetworking && engine->m_bServer)
        {
            engine->netwBridg_->RespawnPlayers(engine->gameWorld_, *engine->assMan_);
        }
        // Camara
        if (key == GLFW_KEY_F11 && action == GLFW_PRESS) 
        {
            engine->settings_.thirdPerson = !engine->settings_.thirdPerson;
        }
        // ImGui
        if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
            engine->settings_.imGui = !engine->settings_.imGui;
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
	fg.time 	  = static_cast<float>(glfwGetTime());

    fg.skyBox = settings_.skyBox;
    fg.grass = settings_.grass;

	for (auto& [id, model] : models)
	{
		if (gameWorld_.IsShot(id)) {
			PointLight pl;
			pl.position = model.GetInterpolatedPosition();
			fg.pointLights.push_back(pl);
		}
	}

	RenderList rl;
    uint16_t hbm;
	for (auto& [id, model] : models)
	{
		glm::mat4 modelProjection = glm::mat4(1.0f);
		modelProjection = glm::translate(modelProjection, model.GetInterpolatedPosition());
		modelProjection *=  glm::mat4_cast(glm::quat(model.GetInterpolatedRotation()));
		modelProjection = glm::scale(modelProjection, model.GetScale());

        
		for (auto& mesh : model.GetMeshes())
		{
			DrawCommand dc;

			dc.mesh = mesh.get();
			dc.material = GetMaterial(mesh->GetMaterialId());
			dc.transform = modelProjection;
			dc.tint = {1,1,1,1};
			dc.renderPass = settings_.debugView ? RenderPass::Debug : RenderPass::Opaque;
            
			rl.commands.push_back(dc);

            // ToDo: Update when refactoring BuildRenderList. Clean enough for now
            hbm = mesh->GetHitboxMaterialId();
		}

		if (settings_.hitboxes)
		{
			DrawCommand dc;

			dc.mesh = model.GetHitboxMesh();
			dc.material = GetMaterial(hbm);
			
			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetInterpolatedPosition());
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
			dc.material = GetMaterial(hbm);

			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetInterpolatedPosition());
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
			dc.material = GetMaterial(hbm);

			glm::mat4 modelMat = glm::mat4(1.0f);
    		modelMat = glm::translate(modelMat, model.GetInterpolatedPosition());
    		modelMat = glm::scale(modelMat, glm::vec3(model.GetRadius()));

			dc.transform = modelMat;
			dc.tint = {1,1,1,1};
			dc.renderPass = RenderPass::Opaque;

			rl.commands.push_back(dc);
		}
    }

    if (settings_.terrain)
    {
        std::vector<DrawCommand> dcs = terrainHandler_->BuildDrawCommands(settings_.debugView ? RenderPass::Debug : RenderPass::Opaque);
        rl.commands.append_range(dcs);
    }

    return std::tuple(rl, fg);
};

void Engine::HandleImGui(int step)
{
    if (step != 0 && !settings_.imGui) return;

    switch (step)
    {
        case 0:
        {
            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window_->Get(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
            ImGui_ImplOpenGL3_Init();
            break;
        }
        case 1:
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Settings");
            ImGui::Checkbox("Grass", &settings_.grass);
            ImGui::Checkbox("Terrain", &settings_.terrain);
            ImGui::Checkbox("Skybox", &settings_.skyBox);
            ImGui::Checkbox("Third person", &settings_.thirdPerson);
            ImGui::Checkbox("Predictive client", &settings_.predictiveClient);
            ImGui::Checkbox("Adjust camera", &settings_.adjustCamera);
            ImGui::Checkbox("Mouse looking", &settings_.mouseLooking);
            ImGui::Checkbox("Debug view", &settings_.debugView);
            ImGui::Checkbox("Hitboxes", &settings_.hitboxes);
            ImGui::Checkbox("Bloom", &settings_.bloom);
            ImGui::Checkbox("Simple flight", &settings_.simpleFlight);
            ImGui::NewLine();

            if (netwBridg_->GetRole() == NetworkBridge::Role::Client)
            {
                float delayMs = netwBridg_->GetRenderDelay() * 1000.0f;
                if (ImGui::SliderFloat("RenderDelay (ms)", &delayMs, 0.0f, 300.0f))
                    netwBridg_->SetRenderDelay(delayMs / 1000.0f);
            }
            bool changed = false;
            changed |= ImGui::SliderInt("Fake lag (ms)", &settings_.fakeLag, 0, 300);
            changed |= ImGui::SliderFloat("Fake package loss (%)", &settings_.pkgLossPct, 0.0f, 20.0f);
            changed |= ImGui::SliderFloat("Fake jitter (%)", &settings_.pkgJitter, 0, 20.0f);
            
            if (changed)
            {
                if (netwBridg_->GetRole() == NetworkBridge::Role::Client)
                    ClientTransport::SetFakeNetwork(settings_.fakeLag, settings_.pkgLossPct, settings_.pkgJitter);
                else if (netwBridg_->GetRole() == NetworkBridge::Role::Server)
                    ServerTransport::SetFakeNetwork(settings_.fakeLag, settings_.pkgLossPct, settings_.pkgJitter);
            }

            ImGui::NewLine();

            if (ImGui::CollapsingHeader("Stats"))
            {
                if (playerId_ != 0 && gameWorld_.GetScene().ModelExists(playerId_))
                {
                    auto& model = gameWorld_.GetScene().GetModelByReference(playerId_);
                    glm::vec3 pos = model.GetPosition();
                    glm::vec3 forward = model.GetForward();
                    
                    float angle = atan2(forward.z, forward.x) * 180.0f / M_PI; // Convert to degrees
                    
                    ImGui::Text("Ship Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                    ImGui::Text("Ship Angle: %.2f degrees", angle);
                }

                for (const auto& [name, stat] : debugStats_.AllStats())
                {
                    if (stat.isCounter)
                        ImGui::Text("%s: %.1f/s (total %llu)", name.c_str(), stat.last.rate, (unsigned long long) stat.total);
                    else
                        ImGui::Text("%s: mean %.4f, min %.4f, max %.4f", name.c_str(), stat.last.mean, stat.last.min, stat.last.max);
                    
                    int offset = stat.histCount < Stat::HISTORY_DURATION ? 0 : stat.histHead;
                    ImGui::PlotLines(("##" + name).c_str(), stat.hist.data(), stat.histCount, offset,
                                      nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 40));
                }
            }

            ImGui::End();
            //ImGui::ShowDemoWindow();
            break;
        }
        case 2:
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            break;
        }
        case 3:
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            break;
        }
        default:
            break;
    }
}
