#pragma once

#include "../../../engine/rendering/terrain/chunk-generator.h"

class BakedMapGenerator : public IChunkGenerator
{
public:
    BakedMapGenerator(float verticalOffset);
    ~BakedMapGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    glm::vec3 NormalAt(glm::vec3 position) const override;
    float MinHeight() const override { return minHeight_ + verticalOffset_; };
    float MaxHeight() const override { return maxHeight_ + verticalOffset_; };
private:
    std::vector<float> vData_;
    int width_, height_;
    float worldSize_, minHeight_, maxHeight_, verticalOffset_;

    void ReadMapConfig();
};
