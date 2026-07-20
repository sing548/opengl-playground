#include "player-system.h"

#include "../../engine/systems/system-structs.h"
#include <cmath>

#include "../networking/network-bridge/network-bridge.h"
#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void PlayerSystem::Update(SystemsContext& ctx)
{
    if (ctx.replay || ctx.authoritative)
        ExecuteInput(ctx.dT, ctx.world, ctx.assMan, ctx.current, ctx.previous, ctx.localPlayerId, ctx.authoritative, ctx.settings.simpleFlight, ctx.bridge);
    else if (!ctx.replay && ctx.settings.predictiveClient)
    {
        ExecuteInput(ctx.dT, ctx.world, ctx.assMan, ctx.current, ctx.previous, ctx.localPlayerId, ctx.authoritative, ctx.settings.simpleFlight, ctx.bridge);
        Shoot(ctx.world, ctx.assMan, ctx.localPlayerId, ctx.dT, ctx.current, ctx.previous, true, ctx.bridge);
    }
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
                                bool lockRAndV,
                                NetworkBridge& bridge
                            )
{ 
    
    for (auto& [id, state] : currentInputStates)
    {
        if (!gameWorld.GetScene().ModelExists(id)) continue;
        auto& model = gameWorld.GetScene().GetModelByReference(id);

        auto forward = model.GetForward();

        glm::vec3 pos = model.GetPosition();
        glm::vec3 toTarget(state.aim_x - pos.x, 0.0f, state.aim_z - pos.z);
        glm::vec3 currentXZ(forward.x, 0.0f, forward.z);

        if (glm::length(toTarget) > 1e-4f && glm::length(currentXZ) > 1e-4f)
        {
            glm::vec3 desired = glm::normalize(toTarget);
            glm::vec3 current = glm::normalize(currentXZ);

            float cross = current.z * desired.x - current.x * desired.z;
            float dot   = current.x * desired.x + current.z * desired.z;
            float angle = std::atan2(cross, dot);

            if (std::abs(angle) > 1e-4f)
                RotateModel(id, gameWorld.GetScene(), glm::angleAxis(angle, glm::vec3(0, 1, 0)), false);
        }

        const float acc = dT * 0.15f;
        glm::vec3 speed = model.GetVelocity();

        if (state.forward)
            speed += acc * model.GetForward();

        if (state.left || state.right)
        {
            glm::vec3 right = glm::cross(model.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f));
            if (glm::length(right) > 1e-4f)
            {
                right = glm::normalize(right);
                if (state.right) speed += acc * right;
                if (state.left)  speed -= acc * right;
            }
        }

        model.SetVelocity(speed);

        if (!authoritative) continue;

        Shoot(gameWorld, assMan, id, dT, currentInputStates, previousInputStates, false, bridge);
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

void PlayerSystem::Shoot(GameWorld& gameWorld,
                         AssetManager& assMan,
                         uint32_t playerId,
                         float dT,
                         std::unordered_map<uint32_t, InputState>& current,
                         std::unordered_map<uint32_t, InputState>& previous,
                         bool predicted,
                         NetworkBridge& bridge)
{
    if (!current.contains(playerId) || !previous.contains(playerId) || !gameWorld.IsPlayer(playerId)) return;
    auto& pd = gameWorld.GetPlayerData(playerId);

    if (pd.shotCooldown > 0.0f)
    {
        pd.shotCooldown = std::max(0.0f, pd.shotCooldown - dT);
        return;
    }
    
    bool shootPressed = current.at(playerId).shoot && !previous.at(playerId).shoot;

    if (shootPressed)
        pd.shotCooldown = 0.1f;
    else if (current.at(playerId).shoot)
        pd.shotCooldown = 0.5f;
    else 
        return;

    auto shooter = gameWorld.GetScene().GetModelByReference(playerId);
    PhysicalInfo pi         = PhysicalInfo();
	pi.position_		    = shooter.GetPosition();
	pi.rotation_		    = shooter.GetRotation();
	pi.angularVelocity_	    = shooter.GetRotationSpeed();
	pi.scale_			    = shooter.GetScale();
	pi.velocity_    		= shooter.GetVelocity();
    auto id = spawner::SpawnShot(gameWorld, assMan, pi, playerId, predicted ? localPredCounter++ : 0, predicted, current.at(playerId).tick);

    if (predicted)
        bridge.AddPredictedShot(id);
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
