#include "network-poll-system.h"

#include "../../game-world/game-world.h"
#include "../../../engine/window/window.h"
#include "../../networking/network-bridge/network-bridge.h"

void NetworkPollSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() != NetworkBridge::Role::Client)
        return;

    ctx.world.ClearKilledPlayers();

    ctx.bridge.PollEvents(ctx.world, ctx.assMan);
    auto playerId = ctx.bridge.GetPlayerId();

    if (playerId != 0)
    {
        ctx.localPlayerId = playerId;
        auto state = InputState { playerId, false, false, false, false, false, false  };
        ctx.current.try_emplace(playerId, state);
        ctx.previous.try_emplace(playerId, state);
    }
}
