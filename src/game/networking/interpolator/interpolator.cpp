#include "interpolator.h"

#include <algorithm>

#include "../../../engine/models/scene.h"
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

void Interpolator::InterpolateGameState(Scene& scene, float renderTime, uint32_t playerId, bool predictiveClient)
{
    while (snapshots_.size() >= 2)
    {
        auto& second = snapshots_.at(1);
        
        if (second.gameTime >= renderTime)
            break;

        snapshots_.pop_front();
    }

    if (snapshots_.size() < 2)
    {
        debugStats_["net.underrun"].Hit();
        return;
    } 

    auto& first = snapshots_.at(0);
    auto& second = snapshots_.at(1);

    float deltaTime = second.gameTime - first.gameTime;

    int count = snapshots_.size();
    
    float intFactor = std::clamp((renderTime - first.gameTime) / (second.gameTime - first.gameTime), 0.0f, 1.0f);

    //https://de.wikipedia.org/wiki/Kubisch_Hermitescher_Spline
    float t = intFactor;
    float t2 = t * t;
    float t3 = t * t * t;

    float h00 =  2.0f*t3 - 3.0f*t2 + 1.0f;
    float h10 =       t3 - 2.0f*t2 + t;
    float h01 = -2.0f*t3 + 3.0f*t2;
    float h11 =       t3 -      t2;

    //std::cout << count << " snapshots active! Currently using timestamp: " << first.gameTime << " and " << second.gameTime << ", interpolation factor: " << intFactor << "\n";

    for (auto& [id, oldEnt] : first.entities)
    {
        if (!second.entities.contains(id) || !scene.ModelExists(id)) continue;
        if (id == playerId && predictiveClient) continue;

        auto& newEnt = second.entities.at(id);
        auto& realEnt = scene.GetModelByReference(id);

        glm::vec3 tangent0 = oldEnt.velocity * 60.0f * deltaTime;
        glm::vec3 tangent1 = newEnt.velocity * 60.0f * deltaTime;

        glm::vec3 position = h00 * oldEnt.position
                           + h10 * tangent0
                           + h01 * newEnt.position
                           + h11 * tangent1;

        realEnt.SetPosition(position);
        realEnt.SetScale(glm::mix(oldEnt.scale, newEnt.scale, intFactor));
        realEnt.SetRotation(glm::slerp(oldEnt.rotation, newEnt.rotation, intFactor));
        realEnt.SetVelocity(glm::mix(oldEnt.velocity, newEnt.velocity, intFactor));
        realEnt.SetRotationSpeed(glm::mix(oldEnt.angularVelocity, newEnt.angularVelocity, intFactor));
    }
}
