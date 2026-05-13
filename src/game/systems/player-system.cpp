#include "player-system.h"

#include "../spawner/spawner.h"
#include "../game-world/game-world.h"

void PlayerSystem::Update(float dT,
                          GameWorld& gameWorld,
                          AssetManager& assMan,
                          std::unordered_map<int, InputState>& currentInputStates,
                          std::unordered_map<int, InputState>& previousInputStates,
                          int playerId,
                          bool shoot)
{
    ExecuteInput(dT, gameWorld, assMan, currentInputStates, previousInputStates, playerId, shoot);
    UpdatePlayerData(dT, gameWorld);
}



void PlayerSystem::ExecuteInput(float dT,
                                GameWorld& gameWorld,
                                AssetManager& assMan,
                                std::unordered_map<int, InputState>& currentInputStates,
                                std::unordered_map<int, InputState>& previousInputStates,
                                int playerId,
                                bool shoot)
{
    bool playerExists = gameWorld.GetScene().ModelExists(playerId);

    for (auto& [id, state] : currentInputStates)
    {
        if (!gameWorld.GetScene().ModelExists(id)) continue;

        if (state.left) 
            RotateModel(id, gameWorld.GetScene(), {0.0f, glm::radians(2.1f), 0.0f});

        if (state.right)
            RotateModel(id, gameWorld.GetScene(), {0.0f, glm::radians(-2.1f), 0.0f});

        if (state.forward) 
        {
            float acc = dT * 0.15f;
            glm::vec3 speed = gameWorld.GetScene().GetModelByReference(id).GetSpeed();
            speed.x += acc;

            // Max Speed
            if (0.2f > speed.x)
                gameWorld.GetScene().GetModelByReference(id).SetSpeed(speed);
        } else 
        {
            float acc = dT * 0.15f;
            if (state.backward)
                acc = dT * .6f;
            glm::vec3 speed = gameWorld.GetScene().GetModelByReference(id).GetSpeed();

            if (speed.x > 0) speed.x -= acc;
            if (speed.x < 0) speed.x = 0;

            gameWorld.GetScene().GetModelByReference(id).SetSpeed(speed);
        }

        if (!shoot) continue;

        /*if (state.shootShot && id == playerId)
        {
            Shoot(gameWorld, assMan, id);
        }
        else 
        {*/
            if (state.shootShot && previousInputStates.at(id).shootShot != true)
            {
                previousInputStates.at(id).shootShot = true;
                Shoot(gameWorld, assMan, id);
            }
        //}
    }
}

void PlayerSystem::RotateModel(unsigned int id, Scene& scene, const glm::vec3& change) 
{
    Model& model = scene.GetModelByReference(id);
    model.SetRotation(model.GetRotation() + change);
}

void PlayerSystem::Shoot(GameWorld& gameWorld, AssetManager& assMan, uint32_t shooterId)
{
    auto shooter = gameWorld.GetScene().GetModelByReference(shooterId);
    PhysicalInfo pi = PhysicalInfo();
    pi.baseOrientation_ = shooter.GetBaseOrientation();
	pi.orientation_ 	= shooter.GetOrientation();
	pi.position_		= shooter.GetPosition();
	pi.rotation_		= shooter.GetRotation();
	pi.rotationSpeed_	= shooter.GetRotationSpeed();
	pi.scale_			= shooter.GetScale();
	pi.speed_			= shooter.GetSpeed();
    spawner::SpawnShot(gameWorld, assMan, pi);
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
