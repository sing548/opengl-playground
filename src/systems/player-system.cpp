#include "player-system.h"

void PlayerSystem::Update(float dT,
                          Scene& scene,
                          AssetManager& assMan,
                          std::unordered_map<int, InputState>& currentInputStates,
                          std::unordered_map<int, InputState>& previousInputStates,
                          int playerId,
                          bool shoot)
{
    ExecuteInput(dT, scene, assMan, currentInputStates, previousInputStates, playerId, shoot);
    UpdatePlayerData(dT, scene);
}



void PlayerSystem::ExecuteInput(float dT,
                                Scene& scene,
                                AssetManager& assMan,
                                std::unordered_map<int, InputState>& currentInputStates,
                                std::unordered_map<int, InputState>& previousInputStates,
                                int playerId,
                                bool shoot)
{
    bool playerExists = scene.ModelExists(playerId);
    if (playerId <= 0 || !playerExists) return;

    for (auto& [id, state] : currentInputStates)
    {
        if (state.left) 
            RotateModel(id, scene, {0.0f, glm::radians(2.1f), 0.0f});

        if (state.right)
            RotateModel(id, scene, {0.0f, glm::radians(-2.1f), 0.0f});

        if (state.forward) 
        {
            float acc = dT * 0.15f;
            glm::vec3 speed = scene.GetModelByReference(id).GetSpeed();
            speed.x += acc;

            // Max Speed
            if (0.2f > speed.x)
                scene.GetModelByReference(id).SetSpeed(speed);
        } else 
        {
            float acc = dT * 0.15f;
            if (state.backward)
                acc = dT * .6f;
            glm::vec3 speed = scene.GetModelByReference(id).GetSpeed();

            if (speed.x > 0) speed.x -= acc;
            if (speed.x < 0) speed.x = 0;

            scene.GetModelByReference(id).SetSpeed(speed);
        }

        if (!shoot) return;

        if (state.shootShot && id == playerId)
        {
            Shoot(scene, assMan, id);
        }
        else 
        {
            if (state.shootShot && previousInputStates.at(id).shootShot != true)
            {
                previousInputStates.at(id).shootShot = true;
                Shoot(scene, assMan, id);
            }
        }
    }
}

void PlayerSystem::RotateModel(unsigned int id, Scene& scene, const glm::vec3& change) 
{
    Model& model = scene.GetModelByReference(id);
    model.SetRotation(model.GetRotation() + change);
}

void PlayerSystem::Shoot(Scene& scene, AssetManager& assMan, uint32_t shooterId)
{
    auto shooter = scene.GetModelByReference(shooterId);
    PhysicalInfo pi = PhysicalInfo();
    pi.position_ = shooter.GetPosition() + shooter.GetOrientation();
    pi.rotation_ = shooter.GetRotation();
    pi.scale_ = glm::vec3(0.05f, 0.05f, 0.05f);
    pi.orientation_ = shooter.GetOrientation();
    pi.baseOrientation_ = glm::vec3(1.0f, 0.0f, 0.0f);
    pi.speed_ = shooter.GetSpeed() + glm::vec3(0.15f, 0.0f, 0.0f);

    Model shot(Model::GetModelPath(ModelType::SHOT), pi, assMan, ModelType::SHOT, true, 0.05f);

    scene.AddModel(shot);
}

void PlayerSystem::UpdatePlayerData(float dT, Scene& scene)
{
    // ToDo: Add to Scene
    for (auto& [id, playerData] : scene.GetPlayerData())
    {
        if (playerData.lastHit > 0) playerData.lastHit -= dT;
        if (playerData.lastHit < 0) playerData.lastHit = 0;
        scene.AddOrUpdatePlayerData(playerData);
    }
}
