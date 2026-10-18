#pragma once

#include <glm/glm.hpp>

#include "i-gameplay-system.h"
#include "../input-manager/raw-input-manager.h"

#include "../../game/systems/system-order.h"

class Camera;

class FreeCameraSystem : public IGameplaySystem
{
public:
    FreeCameraSystem(RawInputManager& rawMan) : rawMan_(rawMan) { };
    void Update(SystemsContext& context) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::PreRender; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::FreeCameraSystem); }
private:
    RawInputManager& rawMan_;
    float moveSpeed_ = 1.0f;
    bool mouseLook_ = false;
    bool firstMouse_ = true;
    bool f3downlast_ = false;

    float mouseXSensitivity_ = 0.002f, mouseYSensitivity_ = 0.002f;

    float lastXPos_ = 0.0f, lastYPos_ = 0.0f, lastXOffset_ = 0.0f, lastYOffset_  = 0.0f;

    void ProcessMouseInput(Camera& cam, glm::vec3& front, glm::vec3& right);
};
