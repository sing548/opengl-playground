#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../engine/window/window.h"

class GameWorld;
struct SystemsContext;

class PhysicsSystem {
public:
    void Update(SystemsContext ctx);
private:
    void MoveModels(float dT, GameWorld& gameWorld, bool authoritative);
    void MoveModel(float dT, Scene& Scene, unsigned int id, const glm::vec3& change);
    void CheckHits(GameWorld& gameWorld, bool authoritative, bool predictive);
    bool Collide(const Model& a, const Model& b);
    void TryCollide(Scene& scene, uint32_t idA, uint32_t idB);
    void CorrectZOffset(Scene& scene);
};

#endif
