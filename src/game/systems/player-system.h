#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../engine/systems/i-gameplay-system.h"
#include "../../game/networking/network-bridge/shared-strucs.h"

#include "system-order.h"

class GameWorld;
class NetworkBridge;

struct SystemsContext;

class PlayerSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::PlayerSystem); }
    bool CanReplay() override { return true; }
private:
    uint32_t localPredCounter = 0x8000000;
    uint32_t audibleLocalId_ = 0;
    void ExecuteInput(float dT,
                      GameWorld& gameWorld,
                      AssetManager& assMan,
                      std::unordered_map<uint32_t, InputState>& currentInputStates,
                      std::unordered_map<uint32_t, InputState>& previousInputStates,
                      uint32_t playerId,
                      bool authoritative,
                      bool simpleFlight,
                      NetworkBridge& bridge
                    );
    void UpdatePlayerData(float dT, GameWorld& gameWorld);
    void RotateModel(uint32_t id, Scene& scene, const glm::quat& change, bool lockRotationAndVelocity);
    void Shoot(GameWorld& gameWorld,
               AssetManager& assMan,
               uint32_t playerId,
               float dT,
               std::unordered_map<uint32_t, InputState>& current,
               std::unordered_map<uint32_t, InputState>& previous,
               bool predicted,
               NetworkBridge& bridge);
};

#endif
