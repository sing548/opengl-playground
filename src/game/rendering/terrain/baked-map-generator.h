#pragma once

#include <filesystem>

#include "../../../engine/rendering/terrain/chunk-generator.h"

#include "world-info.h"

class BakedMapGenerator : public IChunkGenerator
{
public:
    BakedMapGenerator(WorldInfo wi);
    ~BakedMapGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    glm::vec3 NormalAt(glm::vec3 position) const override;
    float MinHeight() const override { return minHeight_; };
    float MaxHeight() const override { return maxHeight_; };
private:
    std::vector<float> vData_;
    int width_, height_;
    float worldSize_, minHeight_, maxHeight_;

    void ReadMapConfig(std::filesystem::path path);
};
