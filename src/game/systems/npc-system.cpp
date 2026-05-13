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
    for (auto& [id, npcData] : gameWorld.GetNpcData())
    {
        auto& npc = gameWorld.GetScene().GetModelByReference(id);

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

        npcData.lastShot -= 0.01f;
    }
}
