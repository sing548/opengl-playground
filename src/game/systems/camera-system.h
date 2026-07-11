#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "../../engine/systems/i-gameplay-system.h"

#include "system-order.h"

class CameraSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& context) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PreRender; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::CameraSystem); }
private:
    bool thirdPersonActive_ = false;
};

#endif
