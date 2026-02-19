#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../models/scene.h"

class PhysicsSystem {
public:
    void Update(Scene& scene);
private:
    void MoveModels(Scene& scene);
    void MoveModel(Scene& scene, unsigned int id, const glm::vec3& change);
};

#endif