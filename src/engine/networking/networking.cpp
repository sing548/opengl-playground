#include "networking.h"

#include "../../game/spawner/spawner.h"
#include "../../game/game-world/game-world.h"

Networking::Networking(bool bServer, const Scene& scene, const char *serverAddr)
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

        networkThread_ = std::thread([this, serverAddr]() {
            try
            {
                SteamNetworkingIPAddr addrServer;
                addrServer.Clear();
                addrServer.ParseString(serverAddr);
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
	
	if (server_) server_->Shutdown();

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
            e.type               = static_cast<uint32_t>(model.type_);
            e.radius             = model.GetRadius();
            e.position           = model.GetPosition();
            e.scale              = model.GetScale();
            e.rotation           = model.GetRotation();
            e.velocity           = model.GetVelocity();
            e.angularVelocity    = model.GetRotationSpeed();

            gameState_->createdEntities.push_back(e);
        }
        else
        {
			if (model.type_ == ModelType::PLAYER || model.type_ == ModelType::NPC)
			{
				// ToDo: "Lightweight" state update ---
				EntityState e;
				e.id                = id;
				e.position          = model.GetPosition();
				e.scale             = model.GetScale();
				e.rotation          = model.GetRotation();
				e.velocity          = model.GetVelocity();
				e.angularVelocity   = model.GetRotationSpeed();
	
				gameState_->entities.push_back(e);
			}
        }
    }
}

void Networking::Shutdown()
{
	running_ = false;

	if (server_) server_->Shutdown();
};

void Networking::SendInputState(InputState& state)
{
	state.tick = currentTick;
	client_->SendStateToServer(state);
};

std::unordered_map<int, InputState> Networking::GetInputStates()
{
	return server_->GetLatestInputStates();
}

// ToDo: This needs to change. Logic for this should not be in engine part of project, but game. Engine should just provide data-exchange layer
std::tuple<unsigned int, std::vector<uint32_t>> Networking::UpdateScene(GameWorld& gameWorld, AssetManager& assMan)
{
	GameState gs;
	std::vector<uint32_t> killedPlayers;

	if (client_->pendingStates.size() == 0) return { 0, killedPlayers };
	{
		std::lock_guard lock(client_->gsMutex);
		gs = std::move(client_->pendingStates.front());
		client_->pendingStates.pop_front();
	}

    if (gs.tick == 0) 
		return { 0, killedPlayers };
	if (currentTick > gs.tick) 
		return { gs.playerId, killedPlayers };
	
	if (gameWorld.GetScene().currentTick >= gs.tick) 
		return { gs.playerId, killedPlayers };

	gameWorld.GetScene().currentTick = gs.tick;

	for (uint32_t id : gs.destroyedEntities)
    {
		auto& players = gameWorld.GetPlayerData();
		if (players.contains(id)) 
			killedPlayers.push_back(id);
			
        gameWorld.MarkEntityForDelete(id);
    }

	for (auto &entity : gs.createdEntities)
	{
		PhysicalInfo pi;
		pi.position			= entity.position;
		pi.rotation			= entity.rotation;
		pi.angularVelocity	= entity.angularVelocity;
		pi.scale			= entity.scale;
		pi.velocity			= entity.velocity;
	
		switch (entity.type) 
		{
			case 0: 
				spawner::SpawnPlayer(gameWorld, assMan, pi, entity.id);
				break;
			case 1: 
				spawner::SpawnShotFromNetwork(gameWorld, assMan, pi, entity.id);
				break;
			case 2: 
				spawner::SpawnNpc(gameWorld, assMan, pi, entity.id);
				break;
			default: break;
		}
	}

	for (const auto& entity : gs.entities)
    {
        if (!gameWorld.GetScene().ModelExists(entity.id))
        {
            std::cout << "Entity not loaded: " << entity.id << std::endl;
            continue;
        }

        auto& m = gameWorld.GetScene().GetModelByReference(entity.id);

        m.SetPosition(entity.position);
        m.SetRotation(entity.rotation);
        m.SetRotationSpeed(entity.angularVelocity);
        m.SetScale(entity.scale);
        m.SetVelocity(entity.velocity);
    }
 
	return { client_->playerId_, killedPlayers };
};
