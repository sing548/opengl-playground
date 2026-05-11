#include "physics-system.h"

PhysicsSystem::PhysicsSystem(bool isAuthoratative)
{
    isAuthoratative_ = isAuthoratative;
}

void PhysicsSystem::Update(float dT, Scene& scene, Window& window)
{
    MoveModels(dT, scene);
    CheckHits(scene);
}

void PhysicsSystem::MoveModels(float dT, Scene& scene)
{
    scene.currentFurthestPosition_ = glm::vec3(0.0f, 0.0f, 0.0f);
    for (auto& [id, model] : scene.GetModels())
    {
        glm::vec3 change = model.GetOrientation() * model.GetSpeed().x;
        MoveModel(dT, scene, id, change);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && model.type_ != ModelType::PLAYER)
            scene.MarkModelForDelete(id);
    }
}

void PhysicsSystem::MoveModel(float dT, Scene& scene, unsigned int id, const glm::vec3& change)
{
    Model& model = scene.GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += change;
    model.SetPosition(position);

    if (abs(position.x) > scene.currentFurthestPosition_.x) scene.currentFurthestPosition_.x = abs(position.x);
    if (abs(position.z) > scene.currentFurthestPosition_.z) scene.currentFurthestPosition_.z = abs(position.z);
}

void PhysicsSystem::CheckHits(Scene& scene)
{
    auto playerModels = scene.GetPhysicalModels();

    for (auto& [id, other] : scene.GetModels())
    {
        if (other.type_ == ModelType::PLAYER)
            continue;
        
        glm::vec3 otherPos = other.GetPosition();
        float otherRadius = other.GetRadius();

        for (auto& [playerId, playerRef] : playerModels)
        {
            auto& player = playerRef.get();

            if (playerId == id)
                continue;
                
            glm::vec3 playerPos = player.GetPosition();
            float playerRadius = player.GetRadius();

            float dist = glm::length(otherPos - playerPos);
            float radiusSum = otherRadius + playerRadius;

            if (dist <= radiusSum)
            {

                PlayerData pd = scene.GetPlayerData(playerId);
                pd.id = playerId;
                pd.lastHit = 0.2;
                pd.lifes -= 1;

                // ToDo: Implement
                scene.AddOrUpdatePlayerData(pd);

                if (pd.lifes == 0 && isAuthoratative_)
                {
                    scene.MarkModelForDelete(playerId);
                    scene.RemovePlayerData(playerId);
                }

                if (other.type_ == ModelType::SHOT && isAuthoratative_)
                    scene.MarkModelForDelete(id);
                //glfwSetWindowShouldClose(window.Get(), true);
            }
        }
    }
}