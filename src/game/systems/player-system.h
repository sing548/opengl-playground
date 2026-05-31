#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../game/networking/network-bridge/shared-strucs.h"

class GameWorld;
struct SystemsContext;

class PlayerSystem {
public:
    void Update(SystemsContext ctx);
private:
    void ExecuteInput(float dT,
                      GameWorld& gameWorld,
                      AssetManager& assMan,
                      std::unordered_map<uint32_t, InputState>& currentInputStates,
                      std::unordered_map<uint32_t, InputState>& previousInputStates,
                      uint32_t playerId,
                      bool authoritative,
                      bool simpleFlight
                    );
    void UpdatePlayerData(float dT, GameWorld& gameWorld);
    void RotateModel(uint32_t id, Scene& scene, const glm::quat& change, bool lockRotationAndVelocity);
    void Shoot(GameWorld& gameWorld, AssetManager& assMan, uint32_t shooterId);
};

#endif
