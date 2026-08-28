#pragma once

#include "../../../../engine/rendering/terrain/chunk-generator.h"

#include "../world-info.h"

class HeightFieldGenerator : public IChunkGenerator
{
public:
    ChunkData Generate(const ChunkRegion& region) const override
    {
        return GenerateArea(glm::vec2(region.coord) * region.regionSize, region.regionSize, region.resolution);
    }
    glm::vec3 NormalAt(glm::vec3 pos) const override;
    
    ChunkData GenerateArea(glm::vec2 minCorner, float span, int resolution) const;
};
