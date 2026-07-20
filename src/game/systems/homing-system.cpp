#include "homing-system.h"

#include <cmath>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../../engine/systems/system-structs.h"
#include "../game-world/game-world.h"

namespace
{
    uint32_t FindClosestTarget(GameWorld& world, const glm::vec3& pos, uint32_t ownerId)
    {
        uint32_t bestId = 0;
        float bestDistSq = std::numeric_limits<float>::infinity();
        Scene& scene = world.GetScene();

        auto consider = [&](uint32_t id) {
            if (id == ownerId) return;
            if (!scene.ModelExists(id)) return;
            const glm::vec3 diff = scene.GetModelByReference(id).GetPosition() - pos;
            float d = glm::dot(diff, diff);
            if (d < bestDistSq)
            {
                bestDistSq = d;
                bestId = id;
            }
        };

        for (const auto& [id, _] : world.GetPlayerData()) consider(id);
        for (const auto& [id, _] : world.GetNpcData())    consider(id);

        return bestId;
    }
}

void HomingSystem::Update(SystemsContext& ctx)
{
    if (ctx.replay) return;

    const float turnRate = 4.0f;
    const float thrust   = 0.20f;
    const float maxSpeed = 1.4f;

    Scene& scene = ctx.world.GetScene();

    for (auto& [shotId, data] : ctx.world.GetShotData())
    {
        if (!data.isHoming) continue;
        if (!scene.ModelExists(shotId)) continue;

        Model& shot = scene.GetModelByReference(shotId);
        glm::vec3 velocity = shot.GetVelocity();
        glm::vec3 pos = shot.GetPosition();

        uint32_t targetId = FindClosestTarget(ctx.world, pos, data.ownerId);

        if (targetId != 0)
        {
            glm::vec3 targetPos = scene.GetModelByReference(targetId).GetPosition();
            glm::vec3 toTarget(targetPos.x - pos.x, 0.0f, targetPos.z - pos.z);
            glm::vec3 velXZ(velocity.x, 0.0f, velocity.z);

            if (glm::length(toTarget) > 1e-4f && glm::length(velXZ) > 1e-4f)
            {
                glm::vec3 desired = glm::normalize(toTarget);
                glm::vec3 current = glm::normalize(velXZ);

                float cross = current.z * desired.x - current.x * desired.z;
                float dot   = current.x * desired.x + current.z * desired.z;
                float angle = std::atan2(cross, dot);

                float maxStep = turnRate * ctx.dT;
                angle = std::clamp(angle, -maxStep, maxStep);

                glm::quat spin = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
                velocity = spin * velocity;
                shot.SetRotation(spin * shot.GetRotation());
            }
        }

        glm::vec3 forward = shot.GetForward();
        velocity += forward * (thrust * ctx.dT);

        float speed = glm::length(velocity);
        if (speed > maxSpeed) velocity *= (maxSpeed / speed);

        shot.SetVelocity(velocity);
    }
}
