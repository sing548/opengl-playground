#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../models/scene.h"
#include "../window/window.h"


class PhysicsSystem {
public:
    PhysicsSystem() = default;
    explicit PhysicsSystem(bool isAuthoritative);

    void Update(float dT, Scene& scene);
private:
    bool isAuthoritative_;
    void MoveModels(float dT, Scene& scene);
    void MoveModel(float dT, Scene& scene, unsigned int id, const glm::vec3& change);
    void CheckHits(Scene& scene);
};

#endif
