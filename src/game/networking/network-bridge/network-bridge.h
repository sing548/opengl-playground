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

class Scene;
class ClientTransport;
class GameWorld;
class ServerTransport;
class AssetManager;

class NetworkBridge
{
public:
    enum class Role { Server, Client, Offline };

    NetworkBridge(Role role, const std::string& serverAddr, int port);
    ~NetworkBridge();
    void PollEvents(GameWorld& world, AssetManager& assMan);
    const Role GetRole() { return role_; };

#pragma region network-debug
    std::tuple<float, float> ReadRenderDrift() 
    {
        auto val = std::tuple<float, float> 
        {
            renderTimeDriftSum_ / (float) renderTimeDriftNum_,
            renderTimeDriftMax_
        };
        renderTimeDriftNum_ = 0;
        renderTimeDriftSum_ = 0;
        renderTimeDriftMax_ = 0;
        return val;
    };
    int GetPendingStateSize() { return pendingStates_.size(); }
#pragma endregion

#pragma region Server
    void ManageGameStateDistribution(GameWorld& gameWorld, float dT);

    // ToDo: return void, add std::vector<std::pair<int, State>>& for perfomance
    // Think about adding bool if guessed (empty queue)
    std::unordered_map<uint32_t, InputState> ConsumeOldestInputStates() {
        constexpr size_t maxDepth = 3;
        std::unordered_map<uint32_t, InputState> states;
        states.reserve(inputQueues_.size());
        
        for (auto& [id, queue] : inputQueues_)
        {
            if (queue.empty())
                continue;

            auto& last = latestInputTickPerPlayer_[id];
            auto it = queue.begin();

            while (queue.size() > 1 && it->second.tick <= last)
                it = queue.erase(it);

            bool shotSink = false;
            while (queue.size() > maxDepth)
            {
                shotSink |= it->second.shoot;
                it = queue.erase(it);
            }

            InputState consumed = it->second;
            consumed.shoot |= shotSink;
            states.emplace(id, consumed);
            last = consumed.tick;
            
            if (queue.size() > 1)
                queue.erase(it);
        }

        return states;
    };
    const std::vector<std::pair<uint32_t, int>> GetQueueSizePerPlayer() { 
        std::vector<std::pair<uint32_t, int>> sizes;

        for (auto& [id, queue] : inputQueues_)
            sizes.push_back(std::pair<uint32_t, int>(id, queue.size()));
        
        return sizes;
    };
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
    std::unordered_map<uint32_t, uint32_t> playersToConnections_;
    std::unordered_map<uint32_t, uint32_t> connectionsToPlayers_;

    // Map holding connectionId <-> currentInputState
    //std::unordered_map<uint32_t, InputState> inputStates_;

#pragma region Server
    std::vector<uint32_t> pendingAdded_;
    std::vector<uint32_t> pendingRemoved_;
    std::map<uint32_t, GameState> pastStates_;
    std::unordered_map<uint32_t, uint32_t> latestInputTickPerPlayer_;

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

    int   renderTimeDriftNum_ = 0;
    float renderTimeDriftSum_ = 0;
    float renderTimeDriftMax_ = 0;

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

    Interpolator inter_ = Interpolator(tickRate_);

    void PollInternalClient();
    
#pragma endregion
};

#endif
