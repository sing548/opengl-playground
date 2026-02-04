#include "networking.h"

#ifndef ASSETS_DIR
#define ASSETS_DIR "./assets"
#endif

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
		
        networkThread_ = std::thread([this]() {
            try {
                server_->ServerLoop(5001, running_);
            } catch (const std::exception& e) {
                std::cerr << "Error in thread: " << e.what() << std::endl;
            }
        });
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

};

void Networking::SendGameState(const Scene& scene, float dT)
{
	tickTimer += dT;

	if (tickTimer >= tickRate)
	{
		currentTick++;
		tickTimer -= tickRate;
		auto gs = BuildGameState(scene);
		server_->UpdateGameState(std::move(gs));
	}
}

std::unique_ptr<GameState> Networking::BuildGameState(const Scene& scene)
{
    auto gs = std::make_unique<GameState>();
    gs->tick = currentTick;

    for (auto& mw : scene.GetModels())
    {
        EntityState e;
        e.id = mw.Id;
		e.position_ 		= mw.model.GetPosition();
		e.scale_			= mw.model.GetScale();
		e.rotation_			= mw.model.GetRotation();
		e.orientation_		= mw.model.GetOrientation();
		e.baseOrientation_	= mw.model.GetBaseOrientation();
		e.speed_			= mw.model.GetSpeed();
		e.rotationSpeed_	= mw.model.GetRotationSpeed();

        gs->entities.push_back(e);
    }

    return gs;
}

void Networking::Shutdown()
{
	running_ = false;
};

void Networking::SendInputState(const InputState& state)
{

};

const GameState& Networking::RetrieveGameState() const
{
    if (!gameState_)
    {
        throw std::runtime_error("No GameState available");
    }
	return *gameState_;
};

unsigned int UpdateScene(const GameState& gs, Scene& scene, uint32_t tick, AssetManager& assMan)
{
	if (tick > gs.tick) return  gs.playerId;

	for (auto &entity : gs.createdEntities)
	{
		PhysicalInfo pi = PhysicalInfo();
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
			case 1: type = ModelType::NPC;		break;
			case 2: type = ModelType::OBJECT;	break;	
		}

		Model model(std::string(ASSETS_DIR) + entity.path, pi, assMan, type, true, entity.radius);
		scene.AddModel(model);
	}

	return gs.playerId;
};
