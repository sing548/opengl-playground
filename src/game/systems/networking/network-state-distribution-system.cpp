#include "network-state-distribution-system.h"

#include "../../game-world/game-world.h"
#include "../../../engine/window/window.h"
#include "../../networking/network-bridge/network-bridge.h"


void NetworkStateDistributionSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() == NetworkBridge::Role::Server)
        ctx.bridge.ManageGameStateDistribution(ctx.world, ctx.dT);
}