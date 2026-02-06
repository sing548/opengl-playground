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

void Networking::SendGameState(const Scene& scene, float dT)
{
	const auto now = std::chrono::steady_clock::now();

	tickTimer += dT;

	if (tickTimer >= tickRate)
	{
		timesSent++;
		currentTick++;
		tickTimer -= tickRate;
		BuildGameState(scene);
		server_->UpdateGameState(std::move(gameState_));

		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime_).count() >= 1)
        {
            std::cout
                << "Updated GameStet: " << timesSent << " times last second, "
                << std::endl;

			timesSent = 1;
            lastLogTime_ = now;
        }
	}
}

void Networking::BuildGameState(const Scene& scene)
{
    gameState_ = std::make_unique<GameState>();
    gameState_->tick = currentTick;

    for (auto& mw : scene.GetModels())
    {
        EntityCreationState e;
        e.id = mw.Id;
		e.type				= ModelType::NPC;
		e.radius			= 0.7f;
		e.path 				= mw.model.GetPath();
		e.position_ 		= mw.model.GetPosition();
		e.scale_			= mw.model.GetScale();
		e.rotation_			= mw.model.GetRotation();
		e.orientation_		= mw.model.GetOrientation();
		e.baseOrientation_	= mw.model.GetBaseOrientation();
		e.speed_			= mw.model.GetSpeed();
		e.rotationSpeed_	= mw.model.GetRotationSpeed();

        gameState_->entities.push_back(e);
    }
}

void Networking::Shutdown()
{
	running_ = false;
};

void Networking::SendInputState(const InputState& state)
{

};

unsigned int Networking::UpdateScene(Scene& scene, AssetManager& assMan)
{
	auto& gs = client_->GetLatestGameState();

	if (currentTick > gs.tick) return  gs.playerId;

	for (auto &entity : gs.entities)
	{
		currentTick = gs.tick;
		
		auto& models = scene.GetModels();

		if (models.size() == 0)
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
	
			Model model(entity.path, pi, assMan, type, true, entity.radius);
			scene.AddModel(model);
		}
		else if (models.size() == 1)
		{
			auto& m = scene.GetModelByReference(entity.id);

			m.SetBaseOrientation(entity.baseOrientation_);
			m.SetOrientation(entity.orientation_);
			m.SetPosition(entity.position_);
			m.SetRotation(entity.rotation_);
			m.SetRotationSpeed(entity.rotationSpeed_);
			m.SetScale(entity.scale_);
			m.SetSpeed(entity.speed_);
			
		}

	}

	return gs.playerId;
};
