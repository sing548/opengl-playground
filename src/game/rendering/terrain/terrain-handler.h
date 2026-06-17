#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "../../../engine/rendering/terrain/chunk-handler.h"

struct FrameGlobals;
class IChunkGenerator;

class TerrainHandler
{
private:
struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const
    {
        return (uint64_t)(uint32_t)v.x | ((uint64_t)(uint32_t)v.y << 32);
    }
};

public:
struct TerrainCollision {
    bool collided;
    float penetration;
    glm::vec3 normal;
};

struct Chunk {
    std::shared_ptr<Mesh> mesh;
    int lod;
};

    TerrainHandler();
    void HandleChunksForArea(const glm::ivec2& area);
    const std::unordered_map<glm::ivec2, Chunk, IVec2Hash>& GetChunks() const { return chunks_; }
    const float GetRegionSize() const { return regionSize_; };
    const int GetRegionResolution() const { return regionResolution_; }; 
    TerrainCollision CheckCollision(glm::vec3 pos, float radius);

private:
    const int renderArea_ = 10;
    const int lowLodArea_ = 7;
    const float regionSize_ = 30.0f;
    const int regionResolution_ = 64;
    const int lowLodRegionResolution_ = 16;
    const int hysteresis_ = 5;

    std::unordered_map<glm::ivec2, Chunk, IVec2Hash> chunks_;
    ChunkHandler chunkHandler_;
    std::unique_ptr<IChunkGenerator> chunkGenerator_;
};

#endif
