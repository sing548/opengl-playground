#include "physics-system.h"

#include "../../engine/systems/system-structs.h"
#include "../../engine/rendering/terrain/i-terrain-handler.h"

#include "../game-world/game-world.h"

void PhysicsSystem::Update(SystemsContext& ctx)
{
    if (!ctx.replay)
    {
        MoveModels(ctx);

        if (!ctx.authoritative && ctx.settings.predictiveClient && ctx.world.GetScene().ModelExists(ctx.localPlayerId))
        {
            auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
            MoveModel(ctx, ctx.localPlayerId);
        }

        CheckHits(ctx.world, ctx.terrainHandler, ctx.authoritative, ctx.settings.predictiveClient);

        if (!ctx.settings.flight3d)
            ClampYOffset(ctx.world.GetScene());
    }
    else
    {
        if (!ctx.world.GetScene().ModelExists(ctx.localPlayerId)) return;
        auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        MoveModel(ctx, ctx.localPlayerId);
    }
}

void PhysicsSystem::MoveModels(SystemsContext& ctx)
{
    ctx.world.GetScene().currentFurthestPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    if (ctx.authoritative)
    for (auto& [id, model] : ctx.world.GetScene().GetModels())
    {
        MoveModel(ctx, id);

        auto position = model.GetPosition();
        if ((abs(position.x) > 80 || abs(position.z) > 80) && !ctx.world.IsPlayer(id))
            ctx.world.MarkEntityForDelete(id);
    }

    else
    for (auto& [id, _] : ctx.world.GetShotData())
    {
        auto& model = ctx.world.GetScene().GetModelByReference(id);
        MoveModel(ctx, id);
    }
}

void PhysicsSystem::MoveModel(SystemsContext& ctx, uint32_t id)
{
    auto& scene = ctx.world.GetScene();
    Model& model = ctx.world.GetScene().GetModelByReference(id);
    glm::vec3 position = model.GetPosition();
    position += model.GetVelocity() * ctx.dT * 60.0f;
    model.SetPosition(position);


    glm::quat rotation = model.GetRotation();
    glm::vec3 rS = model.GetRotationSpeed();
    float w = glm::length(rotation);

    if (w > 1e-6f) {
        glm::vec3 axis = rS / w;
        glm::quat dR   = glm::angleAxis(w * ctx.dT, axis);
        rotation = glm::normalize(dR * rotation);
        model.SetRotation(rotation);
    }

    if (abs(position.x) > scene.currentFurthestPosition.x) scene.currentFurthestPosition.x = abs(position.x);
    if (abs(position.z) > scene.currentFurthestPosition.z) scene.currentFurthestPosition.z = abs(position.z);
}

void PhysicsSystem::CheckHits(GameWorld& gameWorld, ITerrainHandler& terrain, bool authoritative, bool predictive)
{
    if (predictive && !authoritative) return;
    
    Scene& scene = gameWorld.GetScene();

    for (auto& [shotId, shotData] : gameWorld.GetShotData())
    {
        bool shotConsumed = false;
        const Model& shot = scene.GetModelByReference(shotId);

        bool collided = CollideTerrain(terrain, scene, shotId, true);

        if (collided) 
        {
            gameWorld.MarkEntityForDelete(shotId);
            shotConsumed = true;
            continue;
        }

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
            
            if (authoritative)
            {
                npcData.lifes -= 1;
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
    {
        for (auto b = std::next(a); b != players.end(); ++b)
            TryCollide(scene, a->first, b->first);

        if(authoritative && CollideTerrain(terrain, scene, a->first))
        {
            a->second.lastHit = 0.2f;
            a->second.lifes -= 1;
        }
    }
    
    auto& npcs = gameWorld.GetNpcData();
    for (auto a = npcs.begin(); a != npcs.end(); ++a)
    {
        for (auto b = std::next(a); b != npcs.end(); ++b)
            TryCollide(scene, a->first, b->first);

        if(authoritative && CollideTerrain(terrain, scene, a->first))
        {
            a->second.lastHit = 0.2f;
            a->second.lifes -= 1;
        }
    }

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

        float overlap = (modelA.GetRadius() + modelB.GetRadius()) - dist;

        if (overlap > 0)
        {
            modelA.SetPosition(modelA.GetPosition() - n * (overlap * 0.5f));
            modelB.SetPosition(modelB.GetPosition() + n * (overlap * 0.5f));
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

void PhysicsSystem::ClampYOffset(Scene& scene)
{
    for (auto& [id, model] : scene.GetModels())
    {
        auto velo = model.GetVelocity();
        auto pos = model.GetPosition();
        auto rotation = model.GetRotation();
        auto rotationSpeed = model.GetRotationSpeed();

        velo.y = 0.0f;
        model.SetVelocity(velo);
        pos.y = 0.0f;
        model.SetPosition(pos);

        glm::vec3 forward = model.GetForward();
        glm::vec3 flatForward = glm::vec3(forward.x, 0.0f, forward.z);
        
        if (glm::length(flatForward) > 1e-4f) 
        {
            flatForward = glm::normalize(flatForward);
            // ToDo: read this from config - don't assume (0,1,0)
            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 flatRight = glm::cross(flatForward, worldUp);
    
            glm::mat3 B(model.GetBaseRight(), model.GetBaseUp(), model.GetBaseOrientation());
            glm::mat3 W(flatRight, worldUp, flatForward);
            model.SetRotation(glm::quat_cast(W * glm::transpose(B)));
            glm::vec3 rSpeed = model.GetRotationSpeed();
            model.SetRotationSpeed(glm::vec3(rSpeed.x, rSpeed.y, 0.0f));
        }
    }
}

bool PhysicsSystem::CollideTerrain(ITerrainHandler& terrain, Scene& scene, uint32_t id, bool fragile)
{
    bool collision = false;

    auto& model = scene.GetModelByReference(id);
    auto col = terrain.CheckCollision(model.GetPosition(), model.GetRadius());

    glm::vec3 velocity = model.GetVelocity();

    float velocityN = glm::dot(velocity, col.normal);

    const float crashSpeed = 0.5f;

    if (!col.collided) return collision;

    if (!fragile && col.collided && velocityN > -crashSpeed)
    {
        velocity -= glm::min(velocityN, 0.0f) * col.normal;
        model.SetVelocity(velocity);
        model.SetPosition(model.GetPosition() + col.normal * col.penetration);
    }
    else
    {
        collision = true;
        velocity -= 2* velocityN * col.normal;
        model.SetVelocity(velocity);
        model.SetPosition(model.GetPosition() + col.normal * col.penetration);
    }

    return collision;
}
