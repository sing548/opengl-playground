#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <steam/steam_api_common.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "../networking/shared-strucs.h"

class ServerLogic
{
public:
    void ServerLoop(int port, std::atomic<bool>& running);
    uint32_t UpdateGameState(std::unique_ptr<GameState> gs);
    void DistributeGameState(std::atomic<bool>& running);
    std::unordered_map<int, InputState>& GetLatestInputStates();
private:
    struct Client_t
	{
        uint32_t id;
		std::string m_sNick;
	};

    std::unique_ptr<GameState> gameState_;
    std::unordered_map<int, InputState> inputStates_;
    uint32_t shotTick = 0;

    std::mutex mtx_;
    std::condition_variable cv_;

    std::chrono::steady_clock::time_point lastSendTime_;
    std::chrono::duration<double> totalDelta_;
	uint32_t sendCount_ = 0;
    uint32_t deltaCount_ = 0;
    std::chrono::steady_clock::time_point lastLogTime_;
    std::chrono::steady_clock::time_point lastTickTime_;
    
    std::chrono::nanoseconds accumulatedDelta_{0};
    std::chrono::nanoseconds accumulatedTickInterval_{0};
    
    uint64_t deltaSamples_ = 0;
    uint64_t tickSamples_ = 0;

    uint32_t lastSyncedTick_ = 0;

    bool newClientConnected_ = false;
    uint32_t lastConnectedClient_ = UINT16_MAX;
    
    static HSteamNetPollGroup m_hPollGroup;
    static HSteamListenSocket m_hListenSock;
    static ISteamNetworkingSockets *m_pInterface;
    static std::map<HSteamNetConnection, Client_t> m_mapClients;
    static ServerLogic *s_pCallbackInstance;
    
    void SendGameStateToAllClients();
    void SendPackageToClient(unsigned int playerId, const msgpack::sbuffer &buffer);
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