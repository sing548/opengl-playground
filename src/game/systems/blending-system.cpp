#include "blending-system.h"

#include "../networking/network-bridge/network-bridge.h"

#include "../game-world/game-world.h"

void BlendingSystem::Update(SystemsContext& ctx)
{
    bool predictive = ctx.settings.predictiveClient;
    bool networking = ctx.bridge.GetRole() != NetworkBridge::Role::Offline;

    auto& scene = ctx.world.GetScene();

    for (auto& [id, model] : scene.GetModels())
    {
        if (networking)
        {
            if ((id == ctx.localPlayerId && ctx.world.IsPlayer(id) && !predictive) ||
                 ctx.world.IsNpc(id) || ctx.world.IsPlayer(id)) 
            {
                model.SetInterpolatedInfo(model.GetPhysicalInfo());
                continue;
            }
        }

        auto& oldInfo = model.GetPreviousInfo();
        auto curInfo = model.GetPhysicalInfo();

        if (networking && ctx.bridge.GetRole() == NetworkBridge::Role::Client && ctx.world.IsShot(id))
        {
            auto pi = model.GetPhysicalInfo();
            pi.position_ += model.GetVelocity() * ctx.alpha;   // same units logic as the local-player block
            model.SetInterpolatedInfo(pi);
            continue;
        }
        
        curInfo.position_ = glm::mix(oldInfo.position_, curInfo.position_, ctx.alpha);
        curInfo.rotation_ = glm::slerp(oldInfo.rotation_, curInfo.rotation_, ctx.alpha);
        model.SetInterpolatedInfo(curInfo);
    }

    if (ctx.bridge.GetRole() == NetworkBridge::Role::Client && ctx.settings.predictiveClient && scene.ModelExists(ctx.localPlayerId))
    {
        auto& model = scene.GetModelByReference(ctx.localPlayerId);

        auto pi = model.GetPhysicalInfo();
        pi.position_ = pi.position_ + model.GetVelocity() * ctx.alpha;
        //pi.rotation_ = pi.rotation_ * model.GetRotationSpeed() * ctx.alpha;
        model.SetInterpolatedInfo(pi);
    }
}
