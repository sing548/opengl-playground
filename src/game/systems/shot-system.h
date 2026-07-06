#ifndef SHOT_SYSTEM_H
#define SHOT_SYSTEM_H

#include "../../engine/systems/i-gameplay-system.h"

#include "system-order.h"

struct SystemsContext;

class ShotSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::ShotSystem); }
};

#endif
