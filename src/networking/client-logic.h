#ifndef CLIENT_LOGIC_H
#define CLIENT_LOGIC_H

#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <map>
#include <thread>
#include <atomic>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>

#include <steam/isteamnetworkingutils.h>
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


class ClientLogic  {

public:
    ClientLogic();
    ~ClientLogic();
    void ClientLoop(const SteamNetworkingIPAddr &serverAddr, std::atomic<bool>& running);
    const GameState& GetLatestGameState() const;
    private:
    static ClientLogic *s_pCallbackInstance;
    static HSteamNetConnection m_hConnection;
    static ISteamNetworkingSockets *m_pInterface;
    GameState gameState_;


    void PollIncomingMessagesClient(std::atomic<bool>& running);
    //void PollIncomingGameStateMessages(std::atomic<bool>& running);
    void PollConnectionStateChangesClient();
    void OnSteamNetConnectionStatusChangedClient( SteamNetConnectionStatusChangedCallback_t *pInfo, std::atomic<bool>& running );
    
    static void SteamNetConnectionStatusChangedCallbackClient( SteamNetConnectionStatusChangedCallback_t *pInfo, std::atomic<bool>& running )
    {
        s_pCallbackInstance->OnSteamNetConnectionStatusChangedClient( pInfo, running );
    };
};

#endif