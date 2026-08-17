#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "world-info.h"
#include "terrain-config.h"
#include "../../../engine/rendering/materials/material.h"
#include "../../../engine/rendering/terrain/chunk-handler.h"
#include "../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../engine/rendering/terrain/i-terrain-handler.h"

struct DrawCommand;
struct FrameGlobals;

class IChunkGenerator;

class TerrainHandler : public ITerrainHandler
{
public:
    TerrainHandler(std::vector<World> worlds);
    ~TerrainHandler() override;
    void UpdateStreaming(const glm::vec3& observerPos) override;
    TerrainCollision CheckCollision(glm::vec3 pos, float radius) override;
    std::vector<DrawCommand> BuildDrawCommands(RenderPass rp) override;
private:
    ChunkHandler chunkHandler_;
    std::vector<World> worlds_;

    void RefreshChunks(World& world, const glm::ivec2 area);
    void CullChunks(World& world, const glm::ivec2& area);
};

#endif
