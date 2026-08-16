#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "terrain-config.h"
#include "../../../engine/rendering/materials/material.h"
#include "../../../engine/rendering/terrain/chunk-handler.h"
#include "../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../engine/rendering/terrain/i-terrain-handler.h"

struct World;
struct DrawCommand;
struct FrameGlobals;

class IChunkGenerator;

class TerrainHandler : public ITerrainHandler
{
public:
    //TerrainHandler(std::vector<World> worlds);
    TerrainHandler(std::unique_ptr<Material> material, std::unique_ptr<IChunkGenerator> chunkGenerator);
    ~TerrainHandler() override;
    void HandleChunksForArea(const glm::ivec2& area) override;
    TerrainCollision CheckCollision(glm::vec3 pos, float radius) override;
    float GetRegionSize() const override { return TerrainConfig::RegionSize; }; 
    std::vector<DrawCommand> BuildDrawCommands(RenderPass rp) override;
private:
    ChunkHandler chunkHandler_;
    //std::vector<World> worlds_;
    
    std::unique_ptr<Material> material_;
    std::unique_ptr<IChunkGenerator> chunkGenerator_;
    std::unordered_map<glm::ivec2, Chunk, IVec2Hash> chunks_;
    
};

#endif
