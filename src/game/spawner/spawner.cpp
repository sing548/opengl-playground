
#include "spawner.h"

#include "../game-world/game-world.h"

namespace spawner 
{
    uint32_t SpawnShot(GameWorld& gameWorld, AssetManager& assMan, glm::vec3 position,
                       glm::vec3 rotation, glm::vec3 orientation, glm::vec3 speed)
    {
        PhysicalInfo pi = PhysicalInfo();
        pi.position_ = position;
        pi.rotation_ = rotation;
        pi.orientation_ = orientation;
        pi.speed_ = speed + glm::vec3(0.15f, 0.0f, 0.0f);
        
        pi.scale_ = glm::vec3(0.05f);
        pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);

        Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, true, 0.05f);
        auto id = gameWorld.GetScene().AddModel(shot);
        
        // ToDo: Uncomment after implementing system
        //gameWorld.AddShot(id);

        return id;
    }
}