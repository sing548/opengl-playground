#include "networking.h"

Networking::Networking(bool bServer, const Scene& scene)
{
	m_bServer = bServer;
    #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
			FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
	#else
		SteamDatagram_SetAppID( 4888 ); // Just set something, doesn't matter what
		SteamDatagram_SetUniverse( false, k_EUniverseDev );

		SteamDatagramErrMsg errMsg;
		if ( !SteamDatagramClient_Init( errMsg ) )
			FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

		// Disable authentication when running with Steam, for this
		// example, since we're not a real app.
		//
		// Authentication is disabled automatically in the open-source
		// version since we don't have a trusted third party to issue
		// certs.
		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	#endif

	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );

	running_ = true;
    if (bServer)
    {
		server_ = std::make_unique<ServerLogic>();
		gameState_ = std::make_unique<GameState>();
		
		server_->UpdateGameState(std::move(gameState_));

        networkThread_ = std::thread([this]() {
            try 
			{
                server_->ServerLoop(5001, running_);
            }
			catch (const std::exception& e)
			{
                std::cerr << "Error in thread: " << e.what() << std::endl;
            }
        });

		distributionThread_ = std::thread([this]() {
			try 
			{
				server_->DistributeGameState(running_);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error in thread: " << e.what() << std::endl;
			}
		});

        addedModels_ = std::vector<unsigned int>();
        removedModels_ = std::vector<unsigned int>();
    }
    else
    {
		client_ = std::make_unique<ClientLogic>();

        networkThread_ = std::thread([this]() {
            try
            {
                SteamNetworkingIPAddr addrServer;
                addrServer.Clear();
                addrServer.ParseString("127.0.0.1:5001");
				client_->ClientLoop(addrServer, running_);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        });

    }
};

Networking::~Networking()
{
	running_ = false;

	if (networkThread_.joinable())
		networkThread_.join();

	if (distributionThread_.joinable())
		distributionThread_.join();
};

uint32_t Networking::SendGameState(const Scene& scene, const std::vector<unsigned int>& addedModels, const std::vector<unsigned int>& removedModels, float dT)
{
	uint32_t newClientId = 0;
	const auto now = std::chrono::steady_clock::now();

	tickTimer += dT;

    addedModels_.insert(addedModels_.end(), addedModels.begin(), addedModels.end());
    removedModels_.insert(removedModels_.end(), removedModels.begin(), removedModels.end());

	if (tickTimer >= tickRate)
	{
		timesSent++;
		currentTick++;
		tickTimer -= tickRate;
		BuildGameState(scene, addedModels_, removedModels_);
		newClientId = server_->UpdateGameState(std::move(gameState_));

        addedModels_.clear();
        removedModels_.clear();
	}

	if (newClientId != 0)
		return newClientId;

	return 0;
}

void Networking::BuildGameState(const Scene& scene, const std::vector<unsigned int>& addedModels, const std::vector<unsigned int>& removedModels)
{	
    gameState_ = std::make_unique<GameState>();
    gameState_->tick = currentTick;

    // Destroyed entities
    for (unsigned int id : removedModels)
    {
        gameState_->destroyedEntities.push_back(id);
    }

    for (const auto& [id, model] : scene.GetModels())
    {
        const bool isNew = std::find(addedModels.begin(), addedModels.end(), id) != addedModels.end();

        if (isNew)
        {
            // --- Full creation data ---
            EntityCreationState e;
            e.id                 = id;
            e.type               = model.type_;
            e.radius             = model.GetRadius();
            e.position_          = model.GetPosition();
            e.scale_             = model.GetScale();
            e.rotation_          = model.GetRotation();
            e.orientation_       = model.GetOrientation();
            e.baseOrientation_   = model.GetBaseOrientation();
            e.speed_             = model.GetSpeed();
            e.rotationSpeed_     = model.GetRotationSpeed();

            gameState_->createdEntities.push_back(e);
        }
        else
        {
			if (model.type_ == ModelType::PLAYER)
			{
				// ToDo: "Lightweight" state update ---
				EntityState e;
				e.id                 = id;
				e.position_          = model.GetPosition();
				e.scale_             = model.GetScale();
				e.rotation_          = model.GetRotation();
				e.orientation_       = model.GetOrientation();
				e.baseOrientation_   = model.GetBaseOrientation();
				e.speed_             = model.GetSpeed();
				e.rotationSpeed_     = model.GetRotationSpeed();
	
				gameState_->entities.push_back(e);
			}
        }
    }
}

void Networking::Shutdown()
{
	running_ = false;
};

void Networking::SendInputState(const InputState& state)
{
	client_->SendStateToServer(state);
};

std::vector<InputState> Networking::GetInputStates()
{
	return std::vector<InputState>();
}

unsigned int Networking::UpdateScene(Scene& scene, AssetManager& assMan)
{
	GameState gs;

	if (client_->pendingStates.size() == 0) return 0;

	{
		std::lock_guard lock(client_->gsMutex);
		gs = std::move(client_->pendingStates.front());
		client_->pendingStates.pop_front();
	}

    if (gs.tick == 0) 
		return 0;
	if (currentTick > gs.tick) 
		return gs.playerId;
	
	if (scene.currentTick >= gs.tick) 
		return gs.playerId;

	scene.currentTick = gs.tick;

	for (uint32_t id : gs.destroyedEntities)
    {
        scene.RemoveModel(id);
    }

	for (auto &entity : gs.createdEntities)
	{
		PhysicalInfo pi;
		pi.baseOrientation_ = entity.baseOrientation_;
		pi.orientation_ 	= entity.orientation_;
		pi.position_		= entity.position_;
		pi.rotation_		= entity.rotation_;
		pi.rotationSpeed_	= entity.rotationSpeed_;
		pi.scale_			= entity.scale_;
		pi.speed_			= entity.speed_;
	
		ModelType type;
		switch (entity.type) 
		{
			case 0: type = ModelType::PLAYER; 	break;
			case 1: type = ModelType::SHOT;	break;
			default: type = ModelType::SHOT;	break;
		}
	
		Model model(getModelPath(type), pi, assMan, type, true, entity.radius);
		scene.AddModel(model, entity.id);
	}

	for (const auto& entity : gs.entities)
    {
        if (!scene.ModelExists(entity.id))
        {
            std::cout << "Entity not loaded: " << entity.id << std::endl;
            continue;
        }

        auto& m = scene.GetModelByReference(entity.id);

        m.SetBaseOrientation(entity.baseOrientation_);
        m.SetOrientation(entity.orientation_);
        m.SetPosition(entity.position_);
        m.SetRotation(entity.rotation_);
        m.SetRotationSpeed(entity.rotationSpeed_);
        m.SetScale(entity.scale_);
        m.SetSpeed(entity.speed_);
    }
 
	return client_->playerId_;
};
