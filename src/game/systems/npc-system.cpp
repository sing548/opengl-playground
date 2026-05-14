#include "npc-system.h"

#include "../../engine/models/asset-manager.h"

#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

NpcSystem::NpcSystem(bool isAuthoritative)
{
    isAuthoritative_ = isAuthoritative;
}

void NpcSystem::Update(float dT, GameWorld& gameWorld, AssetManager& assMan)
{
    for (auto& [npcId, npcData] : gameWorld.GetNpcData())
    {
        auto& npc = gameWorld.GetScene().GetModelByReference(npcId);

        std::optional<uint32_t> closestPlayer = NULL;

        for (auto& [playerId, playerData] : gameWorld.GetPlayerData())
        {
            if (closestPlayer == NULL) closestPlayer = playerId;

            // ToDo: Build logic to actually search for closest player
        }

        auto& playerModel = gameWorld.GetScene().GetModelByReference(closestPlayer.value());

        auto toPlayer = glm::normalize(playerModel.GetPosition() - npc.GetPosition());
        auto facing  = glm::normalize(npc.GetOrientation());

        auto dot = glm::clamp(glm::dot(facing, toPlayer), -1.0f, 1.0f);
        auto angle = glm::acos(dot);

        if (angle > glm ::radians(1.0f))
        {
            float maxStep   = glm::radians(1.0f); // turn speed per step
            float stepAngle = glm::min(angle, maxStep);
            auto axis = glm::normalize(glm::cross(facing, toPlayer));

            glm::quat current = glm::quat(npc.GetRotation());
            glm::quat delta   = glm::angleAxis(stepAngle, axis);
            glm::quat next    = delta * current;

            npc.SetRotation(glm::eulerAngles(next));
        }


        if (npcData.lastShot <= 0) 
        {
            PhysicalInfo pi = PhysicalInfo();
            pi.baseOrientation_ = npc.GetBaseOrientation();
		    pi.orientation_ 	= npc.GetOrientation();
		    pi.position_		= npc.GetPosition();
		    pi.rotation_		= npc.GetRotation();
		    pi.rotationSpeed_	= npc.GetRotationSpeed();
		    pi.scale_			= npc.GetScale();
		    pi.speed_			= npc.GetSpeed();
            spawner::SpawnShot(gameWorld, assMan, pi); 

            npcData.lastShot = 1.0f;
        }

        npcData.lastShot -= dT;
    }
}
