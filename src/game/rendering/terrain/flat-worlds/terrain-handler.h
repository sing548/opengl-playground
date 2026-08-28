#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <chrono>
#include <future>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "terrain-config.h"
#include "../world-info.h"
#include "../../../../engine/rendering/materials/material.h"
#include "../../../../engine/rendering/terrain/chunk-handler.h"
#include "../../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../../engine/rendering/terrain/i-terrain-handler.h"

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

    std::vector<PendingChunk> pendingNew_;
    std::vector<PendingChunk> pendingUpgrade_;

    std::vector<std::future<PendingAsyncChunk>> asyncChunks_;

    void EnqueueChunks(World& world, const glm::ivec2 area, int worldIdx);
    void DrainQueue(std::chrono::microseconds budget);
    void DrainQueueAsync(std::chrono::microseconds budget);
    void CullChunks(World& world, const glm::ivec2& area);
};

#endif
