#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <map>
#include <thread>
#include <atomic>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

class ServerLogic
{
public:
    void ServerLoop(int port, std::atomic<bool>& running);
private:
    struct Client_t
	{
		std::string m_sNick;
	};

    static HSteamNetPollGroup m_hPollGroup;
    static HSteamListenSocket m_hListenSock;
    static ISteamNetworkingSockets *m_pInterface;
    static std::map<HSteamNetConnection, Client_t> m_mapClients;
    static ServerLogic *s_pCallbackInstance;

    void PollIncomingMessagesServer(std::atomic<bool>& running);
    void PollConnectionStateChangesServer();
    void OnSteamNetConnectionStatusChangedServer( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void SendStringToClient( HSteamNetConnection conn, const char *str );
    void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);

    static void SteamNetConnectionStatusChangedCallbackServer( SteamNetConnectionStatusChangedCallback_t *pInfo )
	{
        s_pCallbackInstance->OnSteamNetConnectionStatusChangedServer( pInfo );
        std::cout << "Something with SteamNetConnectionStatusChangedCallback happened" << pInfo << std::endl;
	};
};

#endif