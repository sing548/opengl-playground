#ifndef NETWORKING_H
#define NETWORKING_H

#include <glm/glm.hpp>

#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <map>
#include <thread>
#include <atomic>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "../networking/client-logic.h"
#include "../networking/server-logic.h"
#include "../models/scene.h"
#include "../networking/shared-strucs.h"

#ifdef _WIN32
	#include <windows.h> // Ug, for NukeProcess -- see below
#else
	#include <unistd.h>
	#include <signal.h>
#endif

#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif


class Networking 
{

public:
    Networking(bool bServer, const Scene& scene);
    ~Networking();

    void SendInputState(InputState& state);
    std::unordered_map<int, InputState>& GetInputStates();
    uint32_t SendGameState(const Scene& scene, const std::vector<unsigned int>& addedModels, const std::vector<unsigned int>& removedModels, float dT);
    unsigned int UpdateScene(Scene& scene, AssetManager& assMan);
    void Shutdown();
    uint32_t currentTick = 0;
private:

    float tickTimer = 0.0f;
    const float tickRate = 1.0f / 30.0f;

    uint32_t timesSent = 0;
    std::chrono::steady_clock::time_point lastLogTime_;

    bool m_bServer;
    InputState inputState_;
    std::unique_ptr<ServerLogic> server_;
    std::unique_ptr<GameState> gameState_;
    std::vector<unsigned int> addedModels_;
    std::vector<unsigned int> removedModels_;

    bool firstLoad_ = true;
    std::unique_ptr<ClientLogic> client_;

    std::thread networkThread_;
    std::thread distributionThread_;
    std::atomic<bool> running_ { false };
    
    void BuildGameState(const Scene& scene, const std::vector<unsigned int>& addedModels, const std::vector<unsigned int>& removedModels);
    static void FatalError( const char *fmt, ... )
    {
    	char text[ 2048 ];
    	va_list ap;
    	va_start( ap, fmt );
    	vsprintf( text, fmt, ap );
    	va_end(ap);
    	char *nl = strchr( text, '\0' ) - 1;
    	if ( nl >= text && *nl == '\n' )
    		*nl = '\0';
    	DebugOutput( k_ESteamNetworkingSocketsDebugOutputType_Bug, text );
    };

    static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
    {
    	/*SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
    	printf( "%10.6f %s\n", time*1e-6, pszMsg );
    	fflush(stdout);
    	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
    	{
    		fflush(stdout);
    		fflush(stderr);
    		NukeProcess(1);
    	}*/
    };
};

#endif
