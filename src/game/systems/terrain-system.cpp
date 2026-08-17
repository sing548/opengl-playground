#include "terrain-system.h"

#include "../../engine/systems/system-structs.h"

#include "../game-world/game-world.h"

void TerrainSystem::Update(SystemsContext& ctx)
{
    if (ctx.settings.terrain && ctx.localPlayerId != 0 && ctx.world.GetScene().ModelExists(ctx.localPlayerId))
    {
        auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        ctx.terrainHandler.UpdateStreaming(model.GetPosition());
    }
}
