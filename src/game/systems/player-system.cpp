#include "player-system.h"

#include "../../engine/systems/system-structs.h"

#include "../networking/network-bridge/network-bridge.h"
#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void PlayerSystem::Update(SystemsContext& ctx)
{
    if (ctx.replay || ctx.authoritative)
        ExecuteInput(ctx);
    else if (!ctx.replay && ctx.settings.predictiveClient)
    {
        ExecuteInput(ctx);
        Shoot(ctx.world, ctx.assMan, ctx.localPlayerId, ctx.dT, ctx.current, ctx.previous, true, ctx.bridge);
    }
    if (!ctx.replay)
    {
        UpdatePlayerData(ctx.dT, ctx.world);
    }
}

void PlayerSystem::ExecuteInput(SystemsContext& ctx)
{ 
    
    for (auto& [id, state] : ctx.current)
    {
        if (!ctx.world.GetScene().ModelExists(id)) continue;
        auto& model = ctx.world.GetScene().GetModelByReference(id);

        auto forward = model.GetForward();

        float rate = 2.1f;

        auto rS = model.GetRotationSpeed();

        rS = state.pitch * rate * model.GetRight() + 
             state.yaw * rate * model.GetUp() +
             state.roll * rate * model.GetForward();

        model.SetRotationSpeed(rS);

        if (state.flightAssist)
        {
            float speed = glm::length(model.GetVelocity());

            if (speed > 1e-5f)
            {
                float assistStrength = 1.0f;
                glm::vec3 dir = model.GetVelocity() / speed;
                glm::vec3 nose = model.GetForward();
                glm::vec3 newDir = glm::normalize(glm::mix(dir, nose, assistStrength));

                model.SetVelocity(newDir * speed);
            }
        }

        if (state.thrust > 0.0f) 
        {
            float acc = ctx.dT * 0.15f;
            glm::vec3 speed = model.GetVelocity();
            speed += acc * model.GetForward();

            // Max Speed - ToDo: Think about re-implementing

            model.SetVelocity(speed);
        } else 
        {
            float acc = ctx.dT * 0.15f;
            //if (state.backward)
            //    acc = dT * .6f;

            glm::vec3 speed = ctx.world.GetScene().GetModelByReference(id).GetVelocity();
            
            float speedLength = glm::length(speed);

            if (speedLength <= acc || speedLength < 0.05)
                speed = glm::vec3(0.0f);
            else if (state.flightAssist)
                speed -= (speed / speedLength) * (acc * 2);

            ctx.world.GetScene().GetModelByReference(id).SetVelocity(speed);
        }

        if (!ctx.authoritative) continue;

        Shoot(ctx.world, ctx.assMan, id, ctx.dT, ctx.current, ctx.previous, false, ctx.bridge);
    }
}

void PlayerSystem::RotateModel(unsigned int id, Scene& scene, const glm::quat& change, SystemsContext& ctx) 
{
    Model& model = scene.GetModelByReference(id);
    model.RotateBy(change);

    if (ctx.current.at(id).flightAssist)
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
