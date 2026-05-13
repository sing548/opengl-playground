#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../engine/window/window.h"

class GameWorld;

class PhysicsSystem {
public:
    PhysicsSystem() = default;
    explicit PhysicsSystem(bool isAuthoritative);

    void Update(float dT, GameWorld& gameWorld);
private:
    bool isAuthoritative_;
    void MoveModels(float dT, Scene& scene);
    void MoveModel(float dT, Scene& Scene, unsigned int id, const glm::vec3& change);
    void CheckHits(GameWorld& gameWorld);
};

#endif
