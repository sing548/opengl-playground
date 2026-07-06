#include "pi-history-system.h"

#include "../networking/network-bridge/network-bridge.h"

#include "../game-world/game-world.h"

void PiHistorySystem::Update(SystemsContext& ctx)
{
    bool predictive = ctx.settings.at("predictive_client");
    bool networking = ctx.bridge.GetRole() != NetworkBridge::Role::Offline;

    auto& scene = ctx.world.GetScene();

    for (auto& [id, model] : scene.GetModels())
    {
        if (networking)
        {
            if (id == ctx.localPlayerId && ctx.world.IsPlayer(id) && !predictive) continue;

            if (ctx.world.IsNpc(id)) continue;
            if (ctx.world.IsPlayer(id)) continue;
        }
        
        model.SetPreviousInfo(model.GetPhysicalInfo());
    }
}
