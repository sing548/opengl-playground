#ifndef NETW_BRIDG_H
#define NETW_BRIDG_H

#include <deque>
#include <map>
#include <span>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <msgpack.hpp>

#include "shared-strucs.h"

class Scene;
class ClientTransport;
class GameWorld;
class ServerTransport;
class AssetManager;

class NetworkBridge
{
public:
    enum class Role { Server, Client };

    NetworkBridge(Role role, const std::string& serverAddr, int port);
    ~NetworkBridge();
    void PollEvents(GameWorld& world, AssetManager& assMan);

#pragma region Server
    void ManageGameStateDistribution(Scene& scene, float dT);
    std::unordered_map<uint32_t, InputState> GetInputStates() { return inputStates_; };
    const uint32_t GetCurrentTick() const { return currentTick_; };
    void RespawnPlayers(GameWorld& world, AssetManager& assMan);
#pragma endregion    

#pragma region Client
    void SendInputState(InputState& state);
    std::tuple<unsigned int, std::vector<uint32_t>> MergeClientWithNetwork(GameWorld& gameWorld, AssetManager& assMan);
#pragma endregion

private:
    Role role_;
    std::unique_ptr<ClientTransport> client_;
    std::unique_ptr<ServerTransport> server_;

    const float tickRate_ = 1.0f / 30.0f;

    float tickTimer_ = 0.0f;

    uint32_t currentTick_ = 0;

    // Map holding modelId <-> connectionId
    std::unordered_map<uint32_t, uint32_t> playersToConnections_;
    std::unordered_map<uint32_t, uint32_t> connectionsToPlayers_;

    std::unordered_map<uint32_t, InputState> inputStates_;

#pragma region Server
    std::vector<uint32_t> pendingRemoved_;
    void PollInternalServer(GameWorld& world, AssetManager& assMan);
    msgpack::sbuffer BuildAndPackGameState(const Scene& scene, bool fullState = false);
    GameState BuildGameState(const Scene& scene, bool fullState = false);
#pragma endregion

#pragma region Client
    uint32_t playerId_ = 0;
    uint32_t previousTick_ = 0;
    std::deque<GameState> pendingStates_;

    void PollInternalClient();
#pragma endregion
};

#endif
