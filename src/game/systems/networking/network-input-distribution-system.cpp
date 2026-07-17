#include "network-input-distribution-system.h"

#include "../../game-world/game-world.h"
#include "../../../engine/window/window.h"
#include "../../networking/network-bridge/network-bridge.h"


void NetworkInputDistributionSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() == NetworkBridge::Role::Client && ctx.localPlayerId > 0)
        ctx.bridge.SendInputState(ctx.current.at(ctx.localPlayerId));
}