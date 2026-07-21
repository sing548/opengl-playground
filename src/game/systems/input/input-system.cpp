#include "input-system.h"

#include "../../game-world/game-world.h"
#include "../../networking/network-bridge/shared-strucs.h"

void InputSystem::Update(SystemsContext& ctx)
{

    if (!ctx.current.contains(ctx.localPlayerId) ||
        ctx.localPlayerId == 0 || std::ranges::find(ctx.world.GetKilledPlayers(), ctx.localPlayerId) != ctx.world.GetKilledPlayers().end()) return;

    ctx.previous.at(ctx.localPlayerId) = ctx.current.at(ctx.localPlayerId);
    
    auto& state = ctx.current.at(ctx.localPlayerId);

    EvaluateFloat(GLFW_KEY_A, GLFW_KEY_D, state.yaw);
    EvaluateFloat(GLFW_KEY_Q, GLFW_KEY_E, state.roll);
    EvaluateFloat(GLFW_KEY_R, GLFW_KEY_F, state.pitch);
    EvaluateFloat(GLFW_KEY_W, GLFW_KEY_S, state.thrust);

    if (rawMan_.GamepadConnected())
    {
        float yaw = RecalcDeadzone(rawMan_.GamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_X).second);
        state.yaw = fabs(yaw) > 0.0f ? -yaw : state.yaw;

        float pitch = RecalcDeadzone(rawMan_.GamepadAxis(GLFW_GAMEPAD_AXIS_LEFT_Y).second);
        state.pitch = fabs(pitch) > 0.0f ? pitch : state.pitch;

        float roll = RecalcDeadzone(rawMan_.GamepadAxis(GLFW_GAMEPAD_AXIS_RIGHT_X).second);
        state.roll = fabs(roll) > 0.0f ? roll : state.roll;

        float thrust = RecalcDeadzone((rawMan_.GamepadAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER).second + 1) * 0.5f);
        state.thrust = fabs(thrust) > 0.0f ? thrust : state.thrust;
    }

    state.shoot = rawMan_.KeyDown(GLFW_KEY_SPACE) || rawMan_.GamepadButtonDown(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
}

void InputSystem::EvaluateFloat(int keyA, int keyB, float& val)
{
    if (rawMan_.KeyDown(keyA) &&
        !rawMan_.KeyDown(keyB))
        val = 1.0f;
    else if (!rawMan_.KeyDown(keyA) &&
             rawMan_.KeyDown(keyB))
        val = -1.0f;
    else
        val = 0.0f;
}

float InputSystem::RecalcDeadzone(float in)
{
    float dz = 0.1;
    float abs = fabs(in);
    return (abs < dz) ? 0.0f : glm::sign(in) * (abs - dz) / (1.0f - dz);
}
