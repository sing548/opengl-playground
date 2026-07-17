#pragma once

#include "../../../engine/systems/i-gameplay-system.h"

#include "../system-order.h"

class NetworkMergeSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PostTick; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::NetworkMergeSystem); }
private:
};
