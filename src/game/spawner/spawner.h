#ifndef SPAWNER_H
#define SPAWNER_H

#include <cstdint>
#include <glm/glm.hpp>

class GameWorld;
class AssetManager;
struct PhysicalInfo;

namespace spawner {
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, glm::vec3 shooterForward = glm::vec3(0.0f), glm::vec3 shooterSpeed = glm::vec3(0.0f), uint32_t id = -1);
    uint32_t SpawnShotFromNetwork(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id);
    uint32_t SpawnNpc(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = -1);
    uint32_t SpawnPlayer(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = -1);
}

#endif
