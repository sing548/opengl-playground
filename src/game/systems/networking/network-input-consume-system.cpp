#include "network-input-consume-system.h"

#include "../../game-world/game-world.h"
#include "../../../engine/window/window.h"
#include "../../networking/network-bridge/network-bridge.h"


void NetworkInputConsumeSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() != NetworkBridge::Role::Server)
        return;

    ctx.world.ClearKilledPlayers();

    ctx.bridge.PollEvents(ctx.world, ctx.assMan);
    auto inputStates = ctx.bridge.ConsumeOldestInputStates();

    for (const auto [id, state] : inputStates)
    {
        if (std::ranges::find(ctx.world.GetKilledPlayers(), id) != ctx.world.GetKilledPlayers().end()) 
            continue;

        auto it = ctx.current.find(state.id);
        
        ctx.previous[state.id] = (it != ctx.current.end()) ? it->second : state;
        ctx.current[state.id] = state;
    }
}
