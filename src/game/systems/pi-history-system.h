#ifndef PI_HISTORY_SYSTEM_H
#define PI_HISTORY_SYSTEM_H

#include "../../engine/systems/i-gameplay-system.h"

#include "system-order.h"

class PiHistorySystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& context) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PreSimulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::PiHistorySystem); }
};

#endif
