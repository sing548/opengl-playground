#include "physics-system.h"

void PhysicsSystem::Update(Scene& scene)
{
    MoveModels(scene);
}

void PhysicsSystem::MoveModels(Scene& scene)
{
    std::vector<unsigned int> deletes;
             
    scene.currentFurthestPosition_ = glm::vec3(0.0f, 0.0f, 0.0f);
    for (auto& [id, model] : scene.GetModels())
    {
        glm::vec3 change = model.GetOrientation() * model.GetSpeed().x;
        MoveModel(scene, id, change);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && model.type_ != ModelType::PLAYER)
            deletes.push_back(id);

        //for (auto id : deletes)
        //    RemoveModel(id);
    }
}

void PhysicsSystem::MoveModel(Scene& scene, unsigned int id, const glm::vec3& change)
{
    Model& model = scene.GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += change;
    model.SetPosition(position);

    if (abs(position.x) > scene.currentFurthestPosition_.x) scene.currentFurthestPosition_.x = abs(position.x);
    if (abs(position.z) > scene.currentFurthestPosition_.z) scene.currentFurthestPosition_.z = abs(position.z);
}
