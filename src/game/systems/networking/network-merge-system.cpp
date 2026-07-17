#include "network-merge-system.h"

#include "../../game-world/game-world.h"
#include "../../../engine/window/window.h"
#include "../../networking/network-bridge/network-bridge.h"


void NetworkMergeSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() != NetworkBridge::Role::Client)
        return;
        
    ctx.bridge.MergeClientWithNetwork(ctx.world, ctx.assMan, ctx.settings.predictiveClient);     
}
