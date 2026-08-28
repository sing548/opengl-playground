#pragma once

#include <filesystem>

#include "../world-info.h"

#include "height-field-generator.h"

class BakedMapGenerator : public HeightFieldGenerator
{
public:
    BakedMapGenerator(WorldInfo wi);
    ~BakedMapGenerator() = default;
    float HeightAt(float x, float z) const override;
    float MinHeight() const override { return minHeight_; };
    float MaxHeight() const override { return maxHeight_; };
private:
    std::vector<float> vData_;
    int width_, height_;
    float worldSize_, minHeight_, maxHeight_;

    void ReadMapConfig(std::filesystem::path path);
};
