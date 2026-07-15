#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <deque>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "../../../engine/metrics/debug-stats.h"

class Scene;
struct EntityState;

class Interpolator
{
public:
    Interpolator(float tickRate, DebugStats& debugStats) : tickRate_(tickRate), debugStats_(debugStats) {};
    ~Interpolator();
    
    void FeedSnapshot(uint32_t tick, const std::vector<EntityState>& entities);
    void InterpolateGameState(Scene& scene, float renderTime, uint32_t playerId, bool predictiveClient);
private:
    
    float tickRate_;
    uint32_t lastInterpretedTick_ = 0;

    DebugStats& debugStats_;

    struct Snapshot {
        uint32_t tick;
        float gameTime;
        std::unordered_map<uint32_t, EntityState> entities;
    };

    std::deque<Snapshot> snapshots_;
};

#endif