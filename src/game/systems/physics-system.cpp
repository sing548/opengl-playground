#include "physics-system.h"

#include "system-structs.h"
#include "../game-world/game-world.h"

void PhysicsSystem::Update(SystemsContext& ctx)
{
    if (!ctx.replay)
    {
        MoveModels(ctx.dT, ctx.world, ctx.authoritative);
        CheckHits(ctx.world, ctx.authoritative, ctx.settings.at("predictive_client"));
        CorrectZOffset(ctx.world.GetScene());
    }
    else
    {
        if (!ctx.world.GetScene().ModelExists(ctx.localPlayerId)) return;
        auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        glm::vec3 change = model.GetVelocity();
        MoveModel(ctx.dT, ctx.world.GetScene(), ctx.localPlayerId, change);
    }
}

void PhysicsSystem::MoveModels(float dT, GameWorld& gameWorld, bool authoritative)
{
    gameWorld.GetScene().currentFurthestPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    if (authoritative)
    for (auto& [id, model] : gameWorld.GetScene().GetModels())
    {
        glm::vec3 change = model.GetVelocity();
        MoveModel(dT, gameWorld.GetScene(), id, change);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && !gameWorld.IsPlayer(id))
            gameWorld.MarkEntityForDelete(id);
    }

    else
    for (auto& [id, _] : gameWorld.GetShotData())
    {
        auto& model = gameWorld.GetScene().GetModelByReference(id);
        MoveModel(dT, gameWorld.GetScene(), id, model.GetVelocity());
    }
}

void PhysicsSystem::MoveModel(float dT, Scene& scene, unsigned int id, const glm::vec3& change)
{
    Model& model = scene.GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += change * dT * 60.0f;
    model.SetPosition(position);

    if (abs(position.x) > scene.currentFurthestPosition.x) scene.currentFurthestPosition.x = abs(position.x);
    if (abs(position.z) > scene.currentFurthestPosition.z) scene.currentFurthestPosition.z = abs(position.z);
}

void PhysicsSystem::CheckHits(GameWorld& gameWorld, bool authoritative, bool predictive)
{
    if (predictive && !authoritative) return;
    
    Scene& scene = gameWorld.GetScene();

    for (auto& [shotId, shotData] : gameWorld.GetShotData())
    {
        bool shotConsumed = false;
        const Model& shot = scene.GetModelByReference(shotId);

        for (auto& [playerId, playerData] : gameWorld.GetPlayerData())
        {
            const Model& player = scene.GetModelByReference(playerId);

            if (!Collide(shot, player)) continue;

            playerData.lastHit = 0.2f;

            if (authoritative)
            {
                playerData.lifes -= 1;
    
                if (playerData.lifes <= 0)
                    gameWorld.MarkEntityForDelete(playerId);
                gameWorld.MarkEntityForDelete(shotId);
            }
            
            shotConsumed = true;
            break;
        }

        if (shotConsumed) continue;
    
        for (auto& [npcId, npcData] : gameWorld.GetNpcData())
        {
            const Model& npc = scene.GetModelByReference(npcId);

            if (!Collide(shot, npc)) continue;

            npcData.lastHit = 0.2f;

            npcData.lifes -= 1;
    
            if (authoritative)
            {
                if (npcData.lifes <= 0)
                    gameWorld.MarkEntityForDelete(npcId);
                gameWorld.MarkEntityForDelete(shotId);
                shotConsumed = true;
            }
            break;
        }
    }

    auto& players = gameWorld.GetPlayerData();
    for (auto a = players.begin(); a != players.end(); ++a)
        for (auto b = std::next(a); b != players.end(); ++b)
            TryCollide(scene, a->first, b->first);

    auto& npcs = gameWorld.GetNpcData();
    for (auto a = npcs.begin(); a != npcs.end(); ++a)
        for (auto b = std::next(a); b != npcs.end(); ++b)
            TryCollide(scene, a->first, b->first);

    for (auto& [playerId, _] : players)
        for (auto& [npcId, _] : npcs)
            TryCollide(scene, playerId, npcId);
}

bool PhysicsSystem::Collide(const Model& a, const Model& b)
{
    float d = glm::length(a.GetPosition() - b.GetPosition());
    return d <= a.GetRadius() + b.GetRadius();
}

void PhysicsSystem::TryCollide(Scene& scene, uint32_t idA, uint32_t idB)
{
    auto& modelA = scene.GetModelByReference(idA);
    auto& modelB = scene.GetModelByReference(idB);

    if (Collide(modelA, modelB))
    {
        glm::vec3 n = modelB.GetPosition() - modelA.GetPosition();
        float dist = glm::length(n);

        if (dist < 1e-6f) return;

        n /= dist;

        float overlap = (modelA.GetRadius() - modelB.GetRadius()) - dist;

        if (overlap > 0)
        {
            modelA.SetPosition(modelA.GetPosition() - n + (overlap * 0.5f));
            modelB.SetPosition(modelB.GetPosition() - n + (overlap * 0.5f));
        }

        glm::vec3 veloA = modelA.GetVelocity();
        glm::vec3 veloB = modelB.GetVelocity();

        auto veloAN = glm::dot(veloA, n);
        auto veloBN = glm::dot(veloB, n);

        if (veloAN - veloBN <= 0) return;

        modelA.SetVelocity(veloA + (veloBN - veloAN) * n);
        modelB.SetVelocity(veloB + (veloAN - veloBN) * n);
    }
}

void PhysicsSystem::CorrectZOffset(Scene& scene)
{
    for (auto& model : scene.GetModels())
    {
        auto velo = model.second.GetVelocity();
        auto pos = model.second.GetPosition();

        if (velo.y > 0 || velo.y < 0) 
        {
            velo.y = 0.0f;
            model.second.SetVelocity(velo);
        }

        if (pos.y > 0 || pos.y < 0)
        {
            pos.y = 0.0f;
            model.second.SetPosition(pos);
        }
    }
}
