
#include "spawner.h"

#include "../game-world/game-world.h"
#include "../../engine/models/model.h"

namespace spawner 
{
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, glm::vec3 shooterForward, glm::vec3 shooterSpeed, uint32_t id)
    {
        glm::vec3 shotBaseOrientation = glm::vec3(1.0f, 0.0f, 0.0f);
        float speedBoost = 0.15f;
        
        pi.position = pi.position + shooterForward;
        pi.velocity = shooterSpeed + shooterForward * speedBoost;   
        pi.scale = glm::vec3(0.05f);

        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, shotBaseOrientation, true, 0.05f);
        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id);

        return id;
    }

    uint32_t SpawnShotFromNetwork(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id)
    {
        glm::vec3 shotBaseOrientation = glm::vec3(1.0f, 0.0f, 0.0f);
        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, shotBaseOrientation, true, 0.05f);
        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id);

        return id;
    }

    uint32_t SpawnNpc(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id)
    {
        Model npc(Model::GetModelPath(ModelType::PLAYER), pi, assMan, ModelType::NPC, glm::vec3(-1.0f, 0.0f, 0.0f));
        id = gameWorld.GetScene().AddModel(npc, id);
        gameWorld.AddNpc(id);

        return id;
    }

    uint32_t SpawnPlayer(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id)
    {
        PlayerData pd = PlayerData();
        Model player(Model::GetModelPath(ModelType::PLAYER), pi, assMan, ModelType::PLAYER, glm::vec3(-1.0f, 0.0f, 0.0f));
        id = gameWorld.GetScene().AddModel(player, id);
        gameWorld.AddPlayer(id, pd);

        return id;
    }
}