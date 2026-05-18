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
    int n = 0;

    for (auto& [npcId, npcData] : gameWorld.GetNpcData())
    {
        auto& npc = gameWorld.GetScene().GetModelByReference(npcId);

        float closestDist = 0.0f;
        std::optional<uint32_t> closestPlayer = NULL;

        for (auto& [playerId, playerData] : gameWorld.GetPlayerData())
        {
            auto& playerModel = gameWorld.GetScene().GetModelByReference(playerId);
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
            if (velocity.x > 0) velocity.x -= dT * 10;
            if (velocity.x < 0) velocity.x += dT * 10;
            if (abs(velocity.x) < 0) velocity.x = 0;

            if (velocity.y > 0) velocity.y -= dT * 10;
            if (velocity.y < 0) velocity.y += dT * 10;
            if (abs(velocity.y) < 0) velocity.x = 0;

            if (velocity.z > 0) velocity.z -= dT * 10;
            if (velocity.z < 0) velocity.z += dT * 10;
            if (abs(velocity.z) < 0) velocity.x = 0;

            if (glm::length(velocity) < 0.05f)
            {
                velocity.x = 0.0f;
                velocity.y = 0.0f;
                velocity.z = 0.0f;
            }

            return;
        }

        auto& playerModel = gameWorld.GetScene().GetModelByReference(closestPlayer.value());

        auto toPlayer = glm::normalize(playerModel.GetPosition() - npc.GetPosition());
        auto facing  = glm::normalize(npc.GetForward());

        auto dot = glm::clamp(glm::dot(facing, toPlayer), -1.0f, 1.0f);
        auto angle = glm::acos(dot);

        if (angle > glm ::radians(1.0f))
        {
            float maxStep   = glm::radians(2.5f);
            float stepAngle = glm::min(angle, maxStep);
            auto axis = glm::normalize(glm::cross(facing, toPlayer));

            glm::quat current = glm::quat(npc.GetRotation());
            glm::quat delta   = glm::angleAxis(stepAngle, axis);
            glm::quat next    = delta * current;

            npc.SetRotation(glm::eulerAngles(next));
        }

        if (angle < glm::radians(90.0f))
        {
            auto oldVelo = npc.GetVelocity();
            auto forward = npc.GetForward();
            auto newVelo = oldVelo + forward * 0.06f * dT;
            npc.SetVelocity(newVelo);
            auto test = npc.GetVelocity();
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
        
        if (npcData.lastHit > 0) npcData.lastHit -= dT;
        if (npcData.lastHit < 0) npcData.lastHit = 0;
        n++;
    }

    if (n == 0)
    {
        PhysicalInfo pi = PhysicalInfo();
        pi.position = glm::vec3(-20.0f, 0.0f, 0.0f);
        pi.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
        pi.scale = glm::vec3(0.2f, 0.2f, 0.2f);
        spawner::SpawnNpc(gameWorld, assMan, pi);
    }
}
