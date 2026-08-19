#pragma once

#include "../../../engine/rendering/terrain/chunk-generator.h"

#include "world-info.h"

class DiskWorldGenerator : public IChunkGenerator
{
public:
    DiskWorldGenerator(WorldInfo wi);
    ~DiskWorldGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    glm::vec3 NormalAt(glm::vec3 position) const override;
    float MinHeight() const override { return worldGenerator_->MinHeight(); };
    float MaxHeight() const override { return worldGenerator_->MaxHeight(); };
private:
    float radius_, edge_, thickness_;
    std::unique_ptr<IChunkGenerator> worldGenerator_;

    float InnerHeight(float x, float z, float h) const;
};
