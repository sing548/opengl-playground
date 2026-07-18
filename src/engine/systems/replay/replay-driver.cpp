#include "replay-driver.h"

#include "../system-structs.h"
#include "../i-gameplay-system.h"

#include "../../../game/networking/network-bridge/shared-strucs.h"

void ReplayDriver::Run(const SystemsContext& originalCtx, const std::map<uint32_t, InputState>& statesToReplay)
{
    SystemsContext ctx = SystemsContext
    {
        fixedDelta_,
        originalCtx.window,
        originalCtx.world,
        originalCtx.assMan,
        originalCtx.bridge,
        originalCtx.terrainHandler,
        replayCurrent_,
        replayPrevious_,
        originalCtx.localPlayerId,
        originalCtx.authoritative,
        true,
        0.0f,
        originalCtx.settings,
        originalCtx.debugStats
    };
    
    for (auto& [tick, state] : statesToReplay)
    {
        ctx.previous[ctx.localPlayerId] = ctx.current[ctx.localPlayerId];
        ctx.current[ctx.localPlayerId] = state;

        for (auto& system : systems_)
        {
            if (!system->CanReplay()) continue;

            system->Update(ctx);
        }
    }
}
