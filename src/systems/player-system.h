#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H

#include "../models/scene.h"
#include "../networking/shared-strucs.h"

class PlayerSystem {
public:
    void Update(float dT,
                Scene& scene,
                AssetManager& assMan,
                std::unordered_map<int, InputState>& currentInputStates,
                std::unordered_map<int, InputState>& previousInputStates,
                int playerId,
                bool shoot);
private:
    void ExecuteInput(float dT,
                      Scene& scene,
                      AssetManager& assMan,
                      std::unordered_map<int, InputState>& currentInputStates,
                      std::unordered_map<int, InputState>& previousInputStates,
                      int plöayerId,
                      bool shoot);
    void UpdatePlayerData(float dT, Scene& scene);
    void RotateModel(unsigned int id, Scene& scene, const glm::vec3& change);
    void Shoot(Scene& scene, AssetManager& assMan, uint32_t shooterId);
};

#endif
