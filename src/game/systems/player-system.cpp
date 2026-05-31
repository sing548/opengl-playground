#include "player-system.h"

#include "system-structs.h"
#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void PlayerSystem::Update(SystemsContext ctx)
{
    if (ctx.replay || ctx.authoritative)
        ExecuteInput(ctx.dT, ctx.world, ctx.assMan, ctx.current, ctx.previous, ctx.localPlayerId, ctx.authoritative, ctx.settings.at("simple_flight"));
    if (!ctx.replay)
    {
        UpdatePlayerData(ctx.dT, ctx.world);
    }
}

void PlayerSystem::ExecuteInput(float dT,
                                GameWorld& gameWorld,
                                AssetManager& assMan,
                                std::unordered_map<uint32_t, InputState>& currentInputStates,
                                std::unordered_map<uint32_t, InputState>& previousInputStates,
                                uint32_t playerId,
                                bool authoritative,
                                bool lockRAndV
                            )
{ 
    
    for (auto& [id, state] : currentInputStates)
    {
        if (!gameWorld.GetScene().ModelExists(id)) continue;
        auto& model = gameWorld.GetScene().GetModelByReference(id);

        if (state.left) 
            RotateModel(id, gameWorld.GetScene(), glm::angleAxis(glm::radians(2.1f), glm::vec3(0, 1, 0)), lockRAndV);

        if (state.right)
            RotateModel(id, gameWorld.GetScene(), glm::angleAxis(glm::radians(-2.1f), glm::vec3(0, 1, 0)), lockRAndV);

        if (state.forward) 
        {
            float acc = dT * 0.15f;
            glm::vec3 speed = model.GetVelocity();
            speed += acc * model.GetForward();

            // Max Speed - ToDo: Think about re-implementing

            model.SetVelocity(speed);
        } else 
        {
            float acc = dT * 0.15f;
            if (state.backward)
                acc = dT * .6f;

            glm::vec3 speed = gameWorld.GetScene().GetModelByReference(id).GetVelocity();

            if (speed.x > 0) speed.x -= acc;
            if (speed.x < 0) speed.x += acc;

            if (speed.y > 0) speed.y -= acc;
            if (speed.y < 0) speed.y += acc;

            if (speed.z > 0) speed.z -= acc;
            if (speed.z < 0) speed.z += acc;

            if (glm::length(speed) < .05) 
            {
                speed.x = 0.0f;
                speed.y = 0.0f;
                speed.z = 0.0f;
            }

            gameWorld.GetScene().GetModelByReference(id).SetVelocity(speed);
        }

        if (!authoritative) continue;

        auto& pd = gameWorld.GetPlayerData(id);

        if (pd.shotCooldown > 0.0f)
            pd.shotCooldown = std::max(0.0f, pd.shotCooldown - dT);
        
        bool shootPressed = state.shoot && !previousInputStates.at(id).shoot;

        if (pd.shotCooldown <= 0.0f)
        {
            if (shootPressed)
            {
                Shoot(gameWorld, assMan, id);
                pd.shotCooldown = 0.1f;
            }
            else if (state.shoot)
            {
                Shoot(gameWorld, assMan, id);
                pd.shotCooldown = 0.5f;
            }
        }
    }
}

void PlayerSystem::RotateModel(unsigned int id, Scene& scene, const glm::quat& change, bool lockRAndV) 
{
    Model& model = scene.GetModelByReference(id);
    model.RotateBy(change);

    if (lockRAndV)
    {
        glm::vec3 velocity = model.GetVelocity();
        velocity = change * velocity;
        model.SetVelocity(velocity);
    }
}

void PlayerSystem::Shoot(GameWorld& gameWorld, AssetManager& assMan, uint32_t shooterId)
{
    auto shooter = gameWorld.GetScene().GetModelByReference(shooterId);
    PhysicalInfo pi         = PhysicalInfo();
	pi.position_		    = shooter.GetPosition();
	pi.rotation_		    = shooter.GetRotation();
	pi.angularVelocity_	    = shooter.GetRotationSpeed();
	pi.scale_			    = shooter.GetScale();
	pi.velocity_    		= shooter.GetVelocity();
    spawner::SpawnShot(gameWorld, assMan, pi, shooterId);
}

void PlayerSystem::UpdatePlayerData(float dT, GameWorld& gameWorld)
{
    // ToDo: Add to Scene
    for (auto& [id, playerData] : gameWorld.GetPlayerData())
    {
        if (playerData.lastHit > 0) playerData.lastHit -= dT;
        if (playerData.lastHit < 0) playerData.lastHit = 0;
    }
}
