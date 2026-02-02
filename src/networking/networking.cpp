#include "networking.h"

Networking::Networking(bool bServer)
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

void Networking::Shutdown()
{
	running_ = false;
};

void Networking::HandInState(InputState state)
{
};

GameState Networking::RetrieveState()
{
};
