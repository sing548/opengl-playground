#ifndef FLAT_CHUNK_GENERATOR
#define FLAT_CHUNK_GENERATOR

#include "height-field-generator.h"

class FlatChunkConfig
{
public:
    static constexpr float HeightScale = 18.0f;
    static constexpr float HeightOffset = -19.0f;
    static constexpr float MinHeight = HeightOffset;
    static constexpr float MaxHeight = HeightOffset + HeightScale;

    static constexpr int Octaves = 5;
    static constexpr float Lacunarity = 2.0f;
    static constexpr float Gain = 0.5f;
    static constexpr float BaseFreq = 0.01f;
};

class FlatChunkGenerator : public HeightFieldGenerator
{
public:
    ~FlatChunkGenerator() = default;
    float HeightAt(float x, float z) const override;
    float MinHeight() const override { return FlatChunkConfig::MinHeight; };
    float MaxHeight() const override { return FlatChunkConfig::MaxHeight; };
};

#endif
