#include "terrain-system.h"

#include "system-structs.h"
#include "../game-world/game-world.h"

void TerrainSystem::Update(SystemsContext& ctx)
{
    if (ctx.settings.at("terrain") && ctx.localPlayerId != 0 && ctx.world.GetScene().ModelExists(ctx.localPlayerId))
    {
        auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        glm::ivec2 area { (int)std::floor(model.GetPosition().x / ctx.terrainHandler.GetRegionSize()), (int)std::floor(model.GetPosition().z / ctx.terrainHandler.GetRegionSize()) };

        if (area == lastArea_) return;
        
        lastArea_ = area;
        ctx.terrainHandler.HandleChunksForArea(area);
    }
}
