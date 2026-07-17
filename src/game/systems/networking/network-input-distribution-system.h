#pragma once

#include "../../../engine/systems/i-gameplay-system.h"

#include "../system-order.h"

class NetworkInputDistributionSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PostSimulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::NetworkInputDistributionSystem); }
private:
};
