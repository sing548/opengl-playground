#include "spawner.h"

#include "../game-world/game-world.h"
#include "../../engine/models/model.h"

namespace spawner 
{
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi,
                       uint32_t shooterId, uint32_t id, bool predicted, uint32_t tick)
    {
        auto& shooter = gameWorld.GetScene().GetModelByReference(shooterId);
        glm::vec3 shotBaseOrientation = glm::vec3(1.0f, 0.0f, 0.0f);
        float speedBoost = 0.15f;
        
        pi.position_ = pi.position_ + shooter.GetForward();
        pi.velocity_ = shooter.GetVelocity() + shooter.GetForward() * speedBoost;   
        pi.scale_ = glm::vec3(0.05f);

        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, shotBaseOrientation, true, 0.05f);
        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id, shooterId, predicted, tick);

        return id;
    }

    uint32_t SpawnHomingShot(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi,
                             uint32_t shooterId, uint32_t id, bool predicted, uint32_t tick)
    {
        auto& shooter = gameWorld.GetScene().GetModelByReference(shooterId);
        glm::vec3 shotBaseOrientation = glm::vec3(1.0f, 0.0f, 0.0f);
        float launchSpeed = 0.32f;

        pi.position_ = pi.position_ + shooter.GetForward();
        pi.velocity_ = shooter.GetForward() * launchSpeed;
        pi.scale_    = glm::vec3(0.15f);

        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, shotBaseOrientation, true, 0.15f);
        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id, shooterId, predicted, tick, true);

        return id;
    }

    uint32_t SpawnShotFromNetwork(GameWorld& gameWorld, AssetManager& assMan, PhysicalInfo pi,
                                  uint32_t shooterId, uint32_t id, float age, bool isHoming)
    {
        glm::vec3 shotBaseOrientation = glm::vec3(1.0f, 0.0f, 0.0f);
        float radius = isHoming ? 0.15f : 0.05f;
        pi.position_ += age * pi.velocity_ * 60.0f;
        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, shotBaseOrientation, true, radius);

        glm::vec3 anchor = pi.position_;
        if (gameWorld.GetScene().ModelExists(shooterId))
        {
            auto& shooter = gameWorld.GetScene().GetModelByReference(shooterId);
            anchor = shooter.GetInterpolatedPosition() + shooter.GetForward();
        }

        shot.SetInterpolationOffset(glm::vec4(pi.position_ - anchor, 1.0f));

        id = gameWorld.GetScene().AddModel(shot, id);
        gameWorld.AddShot(id, shooterId, false, 0, isHoming);

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