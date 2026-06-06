#ifndef FLAT_CHUNK_GENERATOR
#define FLAT_CHUNK_GENERATOR

#include "../../../engine/rendering/terrain/chunk-generator.h"

class FlatChunkGenerator : public IChunkGenerator
{
public:
    ~FlatChunkGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
};

#endif
