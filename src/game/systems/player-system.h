#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../engine/networking/shared-strucs.h"

class GameWorld;

class PlayerSystem {
public:
    void Update(float dT,
                GameWorld& gameWorld,
                AssetManager& assMan,
                std::unordered_map<int, InputState>& currentInputStates,
                std::unordered_map<int, InputState>& previousInputStates,
                int playerId,
                bool shoot);
private:
    void ExecuteInput(float dT,
                      GameWorld& gameWorld,
                      AssetManager& assMan,
                      std::unordered_map<int, InputState>& currentInputStates,
                      std::unordered_map<int, InputState>& previousInputStates,
                      int playerId,
                      bool shoot);
    void UpdatePlayerData(float dT, GameWorld& gameWorld);
    void RotateModel(unsigned int id, Scene& scene, const glm::vec3& change);
    void Shoot(GameWorld& gameWorld, AssetManager& assMan, uint32_t shooterId);
};

#endif
