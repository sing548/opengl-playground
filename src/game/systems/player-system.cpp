#include "player-system.h"

#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void PlayerSystem::Update(float dT,
                          GameWorld& gameWorld,
                          AssetManager& assMan,
                          std::unordered_map<int, InputState>& currentInputStates,
                          std::unordered_map<int, InputState>& previousInputStates,
                          int playerId,
                          bool shoot,
                          const std::map<std::string, bool>& settings)
{
    ExecuteInput(dT, gameWorld, assMan, currentInputStates, previousInputStates, playerId, shoot, settings.at("simple_flight"));
    UpdatePlayerData(dT, gameWorld);
}



void PlayerSystem::ExecuteInput(float dT,
                                GameWorld& gameWorld,
                                AssetManager& assMan,
                                std::unordered_map<int, InputState>& currentInputStates,
                                std::unordered_map<int, InputState>& previousInputStates,
                                int playerId,
                                bool shoot,
                                bool lockRAndV)
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
            if (abs(speed.x) < 0) speed.x = 0;

            if (speed.y > 0) speed.y -= acc;
            if (speed.y < 0) speed.y += acc;
            if (abs(speed.y) < 0) speed.x = 0;

            if (speed.z > 0) speed.z -= acc;
            if (speed.z < 0) speed.z += acc;
            if (abs(speed.z) < 0) speed.x = 0;

            gameWorld.GetScene().GetModelByReference(id).SetVelocity(speed);
        }

        if (!shoot) continue;

        if (state.shootShot && previousInputStates.at(id).shootShot != true)
        {
            previousInputStates.at(id).shootShot = true;
            Shoot(gameWorld, assMan, id);
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
    PhysicalInfo pi     = PhysicalInfo();
	pi.position		    = shooter.GetPosition();
	pi.rotation		    = shooter.GetRotation();
	pi.angularVelocity	= shooter.GetRotationSpeed();
	pi.scale			= shooter.GetScale();
	pi.velocity			= shooter.GetVelocity();
    spawner::SpawnShot(gameWorld, assMan, pi, shooter.GetForward(), shooter.GetVelocity());
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
