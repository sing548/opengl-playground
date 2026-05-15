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
        auto facing  = glm::normalize(npc.GetForward());

        auto dot = glm::clamp(glm::dot(facing, toPlayer), -1.0f, 1.0f);
        auto angle = glm::acos(dot);

        if (angle > glm ::radians(1.0f))
        {
            float maxStep   = glm::radians(1.0f);
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
		    pi.position		= npc.GetPosition();
		    pi.rotation		= npc.GetRotation();
		    pi.angularVelocity	= npc.GetRotationSpeed();
		    pi.scale			= npc.GetScale();
		    pi.velocity			= npc.GetVelocity();
            spawner::SpawnShot(gameWorld, assMan, pi, npc.GetForward()); 

            npcData.lastShot = 1.0f;
        }

        npcData.lastShot -= dT;
    }
}
