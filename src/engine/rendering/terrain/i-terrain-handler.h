#ifndef I_TERRAIN_HANDLER
#define I_TERRAIN_HANDLER

#include <vector>

#include <glm/glm.hpp>

#include "../render-list.h"

class ITerrainHandler 
{
public:
    struct TerrainCollision {
        bool collided;
        float penetration;
        glm::vec3 normal;
    };

    virtual ~ITerrainHandler() = default;
    virtual float GetRegionSize() const = 0; 
    virtual void HandleChunksForArea(const glm::ivec2& area) = 0;
    virtual TerrainCollision CheckCollision(glm::vec3 pos, float radius) = 0;
    virtual std::vector<DrawCommand> BuildDrawCommands(RenderPass rp) = 0;
};

#endif
