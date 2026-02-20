#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../models/scene.h"
#include "../window/window.h"


class PhysicsSystem {
public:
    void Update(float dT, Scene& scene, bool checkHits, Window& window, bool handleDeletes);
private:
    void MoveModels(float dT, Scene& scene);
    void MoveModel(float dT, Scene& scene, unsigned int id, const glm::vec3& change);
    void CheckHits(Scene& scene, bool handleDeletes);
};

#endif
