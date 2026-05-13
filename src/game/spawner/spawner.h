#ifndef SPAWNER_H
#define SPAWNER_H

#include <cstdint>
#include <glm/glm.hpp>

class GameWorld;
class AssetManager;
struct PhysicalInfo;

namespace spawner {
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi,
                       glm::vec3 speedOffset = glm::vec3(0.15f, 0.0f, 0.0f), bool positionOffset = true, uint32_t id = -1);
    uint32_t SpawnNpc(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = -1);
    uint32_t SpawnPlayer(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id = -1);
}

#endif
