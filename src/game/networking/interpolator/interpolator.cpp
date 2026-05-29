#include "interpolator.h"

#include <algorithm>

#include "../../spawner/spawner.h"
#include "../../game-world/game-world.h"
#include "../network-bridge/shared-strucs.h"

void Interpolator::FeedSnapshot(uint32_t tick, const std::vector<EntityState>& entities)
{
    Snapshot snapshot;

    snapshot.tick = tick;
    snapshot.gameTime = tick * tickRate_;
    
    for (auto& entity : entities)
    {
        snapshot.entities.emplace(entity.id, entity);
    }

    snapshots_.push_back(snapshot);
}

Interpolator::~Interpolator() = default;

void Interpolator::InterpolateGameState(Scene& scene, float renderTime)
{
    while (snapshots_.size() >= 2)
    {
        auto& second = snapshots_.at(1);
        
        if (second.gameTime >= renderTime)
        {
            break;
        }

        snapshots_.pop_front();
    }

    if (snapshots_.size() < 2) return;


    auto& first = snapshots_.at(0);
    auto& second = snapshots_.at(1);

    int count = snapshots_.size();
    
    float intFactor = std::clamp((renderTime - first.gameTime) / (second.gameTime - first.gameTime), 0.0f, 1.0f);
    std::cout << count << " snapshots active! Currently using timestamp: " << first.gameTime << " and " << second.gameTime << ", interpolation factor: " << intFactor << "\n";

    for (auto& [id, oldEnt] : first.entities)
    {
        if (!second.entities.contains(id) || !scene.ModelExists(id)) continue;

        auto& newEnt = second.entities.at(id);
        auto& realEnt = scene.GetModelByReference(id);

        realEnt.SetPosition(glm::mix(oldEnt.position, newEnt.position, intFactor));
        realEnt.SetScale(glm::mix(oldEnt.scale, newEnt.scale, intFactor));
        realEnt.SetRotation(glm::slerp(oldEnt.rotation, newEnt.rotation, intFactor));
        realEnt.SetVelocity(glm::mix(oldEnt.velocity, newEnt.velocity, intFactor));
        realEnt.SetRotationSpeed(glm::mix(oldEnt.angularVelocity, newEnt.angularVelocity, intFactor));
    }
}
