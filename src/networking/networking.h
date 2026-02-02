#ifndef NETWORKING_H
#define NETWORKING_H

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

#ifdef _WIN32
	#include <windows.h> // Ug, for NukeProcess -- see below
#else
	#include <unistd.h>
	#include <signal.h>
#endif

#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif


struct InputState {
    bool left      = false;
    bool right     = false;
    bool forward   = false;
    bool backward  = false;
    bool shoot     = false;
};

struct GameState {

};

class Networking 
{

public:
    Networking(bool bServer);
    ~Networking();

    void HandInState(InputState state);
    GameState RetrieveState();
    void Shutdown();
private:

    bool m_bServer;
    InputState inputState_;
    std::unique_ptr<ClientLogic> client_;
    std::unique_ptr<ServerLogic> server_;

    std::thread networkThread_;
    std::atomic<bool> running_ { false };

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
