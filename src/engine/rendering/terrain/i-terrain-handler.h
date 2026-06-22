#ifndef I_TERRAIN_HANDLER
#define I_TERRAIN_HANDLER

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "chunk-handler.h"

class IChunkGenerator;

class ITerrainHandler 
{
protected:
struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const
    {
        return (uint64_t)(uint32_t)v.x | ((uint64_t)(uint32_t)v.y << 32);
    }
};

public:
    struct Chunk {
        std::shared_ptr<Mesh> mesh;
        int lod;
    };
    struct TerrainCollision {
        bool collided;
        float penetration;
        glm::vec3 normal;
    };

    virtual ~ITerrainHandler() = default;
    virtual const float GetRegionSize() const = 0; 
    virtual void HandleChunksForArea(const glm::ivec2& area)  = 0;
    virtual TerrainCollision CheckCollision(glm::vec3 pos, float radius) = 0;

    // ToDo: Remove when refactoring "BuildRenderList" - this is only here as a crutch before the refactor.
    virtual const std::unordered_map<glm::ivec2, Chunk, IVec2Hash>& GetChunks() const = 0;
};

#endif
