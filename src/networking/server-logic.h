#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "../networking/shared-strucs.h"

class ServerLogic
{
public:
    void ServerLoop(int port, std::atomic<bool>& running);
    void UpdateGameState(std::unique_ptr<GameState> gs);
private:
    struct Client_t
	{
        unsigned int id;
		std::string m_sNick;
	};

    std::unique_ptr<GameState> gameState;

    std::mutex mtx;
    
    static HSteamNetPollGroup m_hPollGroup;
    static HSteamListenSocket m_hListenSock;
    static ISteamNetworkingSockets *m_pInterface;
    static std::map<HSteamNetConnection, Client_t> m_mapClients;
    static ServerLogic *s_pCallbackInstance;
    
    void SendGameStateToAllClients();
    void SendGameStateToClient(unsigned int playerId);
    void PollIncomingMessagesServer(std::atomic<bool>& running);
    void PollConnectionStateChangesServer();
    void OnSteamNetConnectionStatusChangedServer( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void SendStringToClient( HSteamNetConnection conn, const char *str );
    void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

    void SetClientNick( HSteamNetConnection hConn, const char *nick, unsigned int id );

    static void SteamNetConnectionStatusChangedCallbackServer( SteamNetConnectionStatusChangedCallback_t *pInfo )
	{
        s_pCallbackInstance->OnSteamNetConnectionStatusChangedServer( pInfo );
        std::cout << "Something with SteamNetConnectionStatusChangedCallback happened" << pInfo << std::endl;
	};
};

#endif