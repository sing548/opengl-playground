#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../game/networking/network-bridge/shared-strucs.h"

class GameWorld;

class PlayerSystem {
public:
    void Update(float dT,
                GameWorld& gameWorld,
                AssetManager& assMan,
                std::unordered_map<int, InputState>& currentInputStates,
                std::unordered_map<int, InputState>& previousInputStates,
                int playerId,
                bool shoot,
                const std::map<std::string, bool>& settings);
private:
    void ExecuteInput(float dT,
                      GameWorld& gameWorld,
                      AssetManager& assMan,
                      std::unordered_map<int, InputState>& currentInputStates,
                      std::unordered_map<int, InputState>& previousInputStates,
                      int playerId,
                      bool shoot,
                      bool simpleFlight);
    void UpdatePlayerData(float dT, GameWorld& gameWorld);
    void RotateModel(unsigned int id, Scene& scene, const glm::quat& change, bool lockRotationAndVelocity);
    void Shoot(GameWorld& gameWorld, AssetManager& assMan, uint32_t shooterId);
};

#endif
