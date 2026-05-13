#ifndef SPAWNER_H
#define SPAWNER_H

class Scene;
class Assetmanager;

namespace spawner {
    uint32_t SpawnShot(Scene& scene, AssetManager& assMan, glm::vec3 position,
                       glm::vec3 rotation, glm::vec3 scale, glm::vec3 orientation,
                       glm::vec3 speed);
    /*uint32_t SpawnNPC(Scene& scene, AssetManager& assMan, glm::vec3 position,
                       glm::vec3 rotation, glm::vec3 scale, glm::vec3 orientation,
                       glm::vec3 speed);*/
}

#endif
