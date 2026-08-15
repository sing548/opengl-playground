#pragma once

#include "../../../engine/rendering/terrain/chunk-generator.h"

class BakedMapGenerator : public IChunkGenerator
{
public:
    BakedMapGenerator();
    ~BakedMapGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    glm::vec3 NormalAt(glm::vec3 position) const override;
private:
    std::vector<uint16_t> vData_;
    int width_, height_, channels_;
    float worldSize_;

    void ReadMapConfig();
};
