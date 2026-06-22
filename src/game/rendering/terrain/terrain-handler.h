#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "terrain-config.h"
#include "../../../engine/rendering/terrain/chunk-handler.h"
#include "../../../engine/rendering/terrain/i-terrain-handler.h"

struct FrameGlobals;
class IChunkGenerator;

class TerrainHandler : public ITerrainHandler
{
public:
    TerrainHandler();
    ~TerrainHandler() override;
    void HandleChunksForArea(const glm::ivec2& area) override;
    TerrainCollision CheckCollision(glm::vec3 pos, float radius) override;
    const float GetRegionSize() const override { return TerrainConfig::RegionSize; }; 
    const std::unordered_map<glm::ivec2, Chunk, IVec2Hash>& GetChunks() const override { return chunks_; };

private:
    std::unordered_map<glm::ivec2, Chunk, IVec2Hash> chunks_;
    ChunkHandler chunkHandler_;
    std::unique_ptr<IChunkGenerator> chunkGenerator_;
};

#endif
