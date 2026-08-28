#pragma once

#include "../world-info.h"
#include "height-field-generator.h"

class DiskWorldGenerator : public HeightFieldGenerator
{
public:
    DiskWorldGenerator(WorldInfo wi);
    ~DiskWorldGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    float MinHeight() const override { return thickness_; };
    float MaxHeight() const override { return thickness_ + 
                                        (worldGenerator_->MaxHeight() - worldGenerator_->MinHeight()); };
private:
    float radius_, edge_, thickness_;
    std::unique_ptr<IChunkGenerator> worldGenerator_;

    float InnerHeight(float x, float z, float h) const;
};
