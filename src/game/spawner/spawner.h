#ifndef SPAWNER_H
#define SPAWNER_H

#include <cstdint>
#include <glm/glm.hpp>

class GameWorld;
class AssetManager;
struct PhysicalInfo;

namespace spawner {
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t shooterId, uint32_t id = 0, bool predicted = false, uint32_t tick = 0);
    uint32_t SpawnShotFromNetwork(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t shooterId, uint32_t id);
    uint32_t SpawnNpc(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = 0);
    uint32_t SpawnPlayer(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = 0);
}

#endif
