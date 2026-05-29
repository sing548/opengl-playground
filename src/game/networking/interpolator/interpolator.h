#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <deque>
#include <vector>
#include <cstdint>
#include <unordered_map>

class Scene;
struct EntityState;

class Interpolator
{
public:
    Interpolator(float tickRate) : tickRate_(tickRate) {};
    ~Interpolator();

    const float lerp_ = 0.1f;
    
    void FeedSnapshot(uint32_t tick, const std::vector<EntityState>& entities);
    void InterpolateGameState(Scene& scene, float renderTime);
private:
    
    float tickRate_;
    uint32_t lastInterpretedTick_ = 0;

    struct Snapshot {
        uint32_t tick;
        float gameTime;
        std::unordered_map<uint32_t, EntityState> entities;
    };

    std::deque<Snapshot> snapshots_;
};

#endif