#pragma once

#include "../../../engine/systems/i-gameplay-system.h"
#include "../../../engine/systems/replay/replay-driver.h"

#include "../system-order.h"

class NetworkReconcileSystem : public IGameplaySystem
{
public:
    NetworkReconcileSystem(ReplayDriver& replayDriver) : replayDriver_(replayDriver) { }
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PostTick; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::NetworkReconcileSystem); }
private:
    ReplayDriver& replayDriver_;
    uint32_t lastReplayedTick_ = 0;
};
