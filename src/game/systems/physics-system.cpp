#include "physics-system.h"

#include "../game-world/game-world.h"

PhysicsSystem::PhysicsSystem(bool isAuthoritative)
{
    isAuthoritative_ = isAuthoritative;
}

void PhysicsSystem::Update(float dT, GameWorld& gameWorld)
{
    MoveModels(dT, gameWorld);
    CheckHits(gameWorld);
}

void PhysicsSystem::MoveModels(float dT, GameWorld& gameWorld)
{
    gameWorld.GetScene().currentFurthestPosition_ = glm::vec3(0.0f, 0.0f, 0.0f);
    for (auto& [id, model] : gameWorld.GetScene().GetModels())
    {
        glm::vec3 change = model.GetOrientation() * model.GetSpeed().x;
        MoveModel(dT, gameWorld.GetScene(), id, change);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && model.type_ != ModelType::PLAYER)
            gameWorld.MarkEntityForDelete(id);
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

void PhysicsSystem::CheckHits(GameWorld& gameWorld)
{
    Scene& scene = gameWorld.GetScene();

    for (auto& [shotId, _] : gameWorld.GetShotData())
    {
        const Model& shot = scene.GetModelByReference(shotId);

        for (auto& [playerId, playerData] : gameWorld.GetPlayerData())
        {
            const Model& player = scene.GetModelByReference(playerId);

            if (!Collide(shot, player)) continue;

            playerData.lastHit = 0.2f;
            playerData.lifes -= 1;

            if (playerData.lifes == 0 && isAuthoritative_)
            {
                gameWorld.MarkEntityForDelete(playerId);
            }
            
            gameWorld.MarkEntityForDelete(shotId);
            
        }
    }

    for (auto& [shotId, _] : gameWorld.GetShotData())
    {
        const Model& shot = scene.GetModelByReference(shotId);

        for (auto& [npcId, npcData] : gameWorld.GetNpcData())
        {
            const Model& npc = scene.GetModelByReference(npcId);

            if (!Collide(shot, npc)) continue;

            npcData.lastHit = 0.2f;
            npcData.lifes -= 1;

            if (npcData.lifes == 0 && isAuthoritative_)
            {
                gameWorld.MarkEntityForDelete(npcId);
            }
            else if (isAuthoritative_)
                gameWorld.MarkEntityForDelete(shotId);
        }
    }
}

bool PhysicsSystem::Collide(const Model& a, const Model& b)
{
    float d = glm::length(a.GetPosition() - b.GetPosition());
    return d <= a.GetRadius() + b.GetRadius();
}
