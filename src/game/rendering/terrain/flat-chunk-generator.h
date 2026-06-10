#ifndef FLAT_CHUNK_GENERATOR
#define FLAT_CHUNK_GENERATOR

#include "../../../engine/rendering/terrain/chunk-generator.h"

class FlatChunkGenerator : public IChunkGenerator
{
public:
    ~FlatChunkGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const;
private:
    const int octaves_ = 5;
    const float lacunatity_ = 2.0f;
    const float gain_ = 0.5f;

    const float baseFreq_ = 0.03f;
    const float heightScale_ = 3.0f;
    const float heightOffset_ = -3.0f;
};

#endif
