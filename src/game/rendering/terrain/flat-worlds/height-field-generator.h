#pragma once

#include "../../../../engine/rendering/terrain/chunk-generator.h"

#include "../world-info.h"

class HeightFieldGenerator : public IChunkGenerator
{
public:
    ChunkData Generate(const ChunkRegion& region) const override;
    glm::vec3 NormalAt(glm::vec3 pos) const override;
};
