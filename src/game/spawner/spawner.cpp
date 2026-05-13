
#include "spawner.h"

#include "../game-world/game-world.h"
#include "../../engine/models/model.h"

namespace spawner 
{
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, glm::vec3 speedOffset, bool positionOffset, uint32_t id)
    {
        if (positionOffset) pi.position_ = pi.position_ + pi.orientation_;
        pi.speed_ = pi.speed_ + speedOffset;   
        pi.scale_ = glm::vec3(0.05f);
        pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);

        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, true, 0.05f);
        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id);

        return id;
    }

    uint32_t SpawnNpc(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id)
    {
        Model npc(Model::GetModelPath(ModelType::PLAYER), pi, assMan, ModelType::NPC);
        id = gameWorld.GetScene().AddModel(npc, id);
        gameWorld.AddNpc(id);

        return id;
    }

    uint32_t SpawnPlayer(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi, uint32_t id)
    {
        PlayerData pd = PlayerData();
        Model player(Model::GetModelPath(ModelType::PLAYER), pi, assMan, ModelType::PLAYER);
        id = gameWorld.GetScene().AddModel(player, id);
        gameWorld.AddPlayer(id, pd);

        return id;
    }
}