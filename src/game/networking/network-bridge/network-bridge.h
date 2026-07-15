#ifndef NETW_BRIDG_H
#define NETW_BRIDG_H

#include <map>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <msgpack.hpp>

#include "shared-strucs.h"
#include "../interpolator/interpolator.h"

#include "../../../engine/metrics/debug-stats.h"

class Scene;
class GameWorld;
class AssetManager;
class ClientTransport;
class ServerTransport;

class NetworkBridge
{
public:
    enum class Role { Server, Client, Offline };

    NetworkBridge(Role role, const std::string& serverAddr, int port, DebugStats& debugStats);
    ~NetworkBridge();
    void PollEvents(GameWorld& world, AssetManager& assMan);
    const Role GetRole() { return role_; };

#pragma region network-debug
    int GetPendingStateSize() { return pendingStates_.size(); }
#pragma endregion

#pragma region Server
    void ManageGameStateDistribution(GameWorld& gameWorld, float dT);

    // ToDo: Think about adding bool if guessed (empty queue)
    std::unordered_map<uint32_t, InputState> ConsumeOldestInputStates();
    const uint32_t GetCurrentTick() const { return currentTick_; };
    void RespawnPlayers(GameWorld& world, AssetManager& assMan);
#pragma endregion    

#pragma region Client
    uint32_t GetPlayerId() { return playerId_; };
    void SendInputState(InputState& state);
    void MergeClientWithNetwork(GameWorld& gameWorld, AssetManager& assMan, bool predictiveClient);
    std::map<uint32_t, InputState>& ResetPlayerToLastInputState(GameWorld& world);
    void AddPredictedShot(uint32_t id) { pendingShotCreations.emplace(currentTick_, id); };
#pragma endregion

private:
    Role role_;
    std::unique_ptr<ClientTransport> client_;
    std::unique_ptr<ServerTransport> server_;

    const float tickRate_ = 1.0f / 30.0f;

    float tickTimer_ = 0.0f;

    // ticks start at 1 in order for queue logic to work.
    uint32_t currentTick_ = 1;

    // Map holding modelId <-> connectionId
    std::unordered_map<uint32_t, uint32_t> connectionsToPlayers_;

    DebugStats& debugStats_;

#pragma region Server
    std::vector<uint32_t> pendingAdded_;
    std::vector<uint32_t> pendingRemoved_;
    std::unordered_map<uint32_t, uint32_t> latestInputTickPerConn_;

    // Idea "stolen" from: https://www.youtube.com/watch?v=8QHHVpBiG-I - Overwatch GDC
    // Input Queue per player. u_map<playerid, map<tick, state>>std::map<uint32_t, InputStat
    std::unordered_map<uint32_t, std::map<uint32_t, InputState>> inputQueues_;

    void PollInternalServer(GameWorld& world, AssetManager& assMan);
    std::tuple<msgpack::sbuffer, msgpack::sbuffer> BuildAndPackGameState(const GameWorld& gameWorld, bool fullState = false);
    std::tuple<GameState, GameState> BuildGameState(const GameWorld& gameWorld, bool fullState = false);
    float CalculateRenderTime();
#pragma endregion

#pragma region Client
    uint32_t playerId_ = 0;
    uint32_t previousTick_ = 0;

    const float renderDelay_ = 0.1f;
    
    std::chrono::steady_clock::time_point timeAtLastTick_;
    std::chrono::steady_clock::time_point lastUpdateTime_;

    EntityState latestPlayerState_;
    uint32_t    latestStateTick_    = 0;
    uint32_t    latestAckTick_      = 0;
    bool        latestStateValid_   = false;

    float serverClock_ = 0.0f;
    bool serverClockInit_ = false;

    std::map<uint32_t, InputState> emptyStates_;
    std::map<uint32_t, InputState> sentInputStates_;

    std::deque<GameState> pendingStates_;

    // <tick, id>
    std::map<uint32_t, uint32_t> pendingShotCreations;

    Interpolator inter_;

    void PollInternalClient();
    
#pragma endregion
};

#endif
