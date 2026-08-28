#include "flat-chunk-generator.h"

#include "fbm-noise.h"
#include "terrain-config.h"

float FlatChunkGenerator::HeightAt(float x, float z) const
{
    return FBMNoise::GenNoise(
                FlatChunkConfig::Octaves,
                FlatChunkConfig::Lacunarity,
                FlatChunkConfig::Gain,
                glm::vec2(x, z) * FlatChunkConfig::BaseFreq) * FlatChunkConfig::HeightScale + FlatChunkConfig::HeightOffset;
}
