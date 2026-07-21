#pragma once

#include "../../../engine/systems/i-gameplay-system.h"
#include "../../../engine/input-manager/raw-input-manager.h"

#include "../system-order.h"

class InputSystem : public IGameplaySystem
{
public:
    InputSystem(RawInputManager& rawMan) : rawMan_(rawMan) {}
    void Update(SystemsContext& context) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Input; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::InputSystem); }
private:
    RawInputManager& rawMan_;

    void EvaluateFloat(int keyA, int keyB, float& val);
    float RecalcDeadzone(float input);
};
