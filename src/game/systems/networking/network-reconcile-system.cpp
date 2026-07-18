#include "network-reconcile-system.h"

#include "../../../engine/window/window.h"

#include "../../game-world/game-world.h"
#include "../../networking/network-bridge/network-bridge.h"

void NetworkReconcileSystem::Update(SystemsContext& ctx)
{
    if (ctx.bridge.GetRole() == NetworkBridge::Role::Client && ctx.settings.predictiveClient)
    {
        bool hasModel = ctx.world.GetScene().ModelExists(ctx.localPlayerId);
        glm::vec3 oldPos {};

        if (hasModel)
            oldPos = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId).GetPosition();
        
        auto& statesToReplay = ctx.bridge.ResetPlayerToLastInputState(ctx.world);
        
        uint32_t maxTick = statesToReplay.empty() ? lastReplayedTick_
                                                  : statesToReplay.rbegin()->first;
        bool newInputThisFrame = (maxTick != lastReplayedTick_);
        lastReplayedTick_ = maxTick;
        
        replayDriver_.Run(ctx, statesToReplay);

        if (hasModel && !newInputThisFrame)
            ctx.debugStats["net.reset_distance"].Add(glm::length(oldPos - 
                ctx.world.GetScene().GetModelByReference(ctx.localPlayerId).GetPosition()));
    }
}
