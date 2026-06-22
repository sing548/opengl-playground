#include "npc-system.h"

#include "../../engine/systems/system-structs.h"

#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void NpcSystem::Update(SystemsContext& context)
{
    int n = 0;

    for (auto& [npcId, npcData] : context.world.GetNpcData())
    {
        n++;
        auto& npc = context.world.GetScene().GetModelByReference(npcId);


        if (npcData.lastHit > 0) npcData.lastHit -= context.dT;
        if (npcData.lastHit < 0) npcData.lastHit = 0;

        if (!context.authoritative) continue;

        float closestDist = 0.0f;
        std::optional<uint32_t> closestPlayer = NULL;

        for (auto& [playerId, playerData] : context.world.GetPlayerData())
        {
            auto& playerModel = context.world.GetScene().GetModelByReference(playerId);
            auto dist = glm::length(playerModel.GetPosition() - npc.GetPosition());

            if (closestPlayer == NULL || dist < closestDist)
            {
                closestDist = dist;
                closestPlayer = playerId;
            }
        }

        if (closestPlayer == NULL) 
        {
            auto velocity = npc.GetVelocity();
            if (velocity.x > 0) velocity.x -= context.dT * 10;
            if (velocity.x < 0) velocity.x += context.dT * 10;

            if (velocity.y > 0) velocity.y -= context.dT * 10;
            if (velocity.y < 0) velocity.y += context.dT * 10;

            if (velocity.z > 0) velocity.z -= context.dT * 10;
            if (velocity.z < 0) velocity.z += context.dT * 10;

            if (glm::length(velocity) < 0.05f)
            {
                velocity.x = 0.0f;
                velocity.y = 0.0f;
                velocity.z = 0.0f;
            }

            continue;
        }

        auto& playerModel = context.world.GetScene().GetModelByReference(closestPlayer.value());

        auto toPlayer = glm::normalize(playerModel.GetPosition() - npc.GetPosition());
        auto facing  = glm::normalize(npc.GetForward());

        auto dot = glm::clamp(glm::dot(facing, toPlayer), -1.0f, 1.0f);
        auto angle = glm::acos(dot);

        if (angle > glm ::radians(1.0f))
        {
            float maxStep   = glm::radians(2.5f);
            float stepAngle = glm::min(angle, maxStep);
            auto axis = glm::normalize(glm::cross(facing, toPlayer));

            glm::quat delta   = glm::angleAxis(stepAngle, axis);
            npc.RotateBy(delta);
        }

        if (angle < glm::radians(90.0f))
        {
            auto oldVelo = npc.GetVelocity();
            auto forward = npc.GetForward();
            auto newVelo = oldVelo + forward * 0.06f * context.dT;
            npc.SetVelocity(newVelo);
            auto test = npc.GetVelocity();
        }


        if (npcData.lastShot <= 0)
        {
            PhysicalInfo pi         = PhysicalInfo();
		    pi.position_		    = npc.GetPosition();
		    pi.rotation_		    = npc.GetRotation();
		    pi.angularVelocity_	    = npc.GetRotationSpeed();
		    pi.scale_			    = npc.GetScale();
		    pi.velocity_			= npc.GetVelocity();
            spawner::SpawnShot(context.world, context.assMan, pi, npcId); 

            npcData.lastShot = 1.0f;
        }

        npcData.lastShot -= context.dT;
    }

    if (n == 0)
    {
        PhysicalInfo pi = PhysicalInfo();
        pi.position_ = glm::vec3(-20.0f, 0.0f, 0.0f);
        pi.rotation_ = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
        pi.scale_ = glm::vec3(0.2f, 0.2f, 0.2f);
        spawner::SpawnNpc(context.world, context.assMan, pi);
    }
}
