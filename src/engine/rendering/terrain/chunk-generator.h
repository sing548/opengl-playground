#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include "chunk-structs.h"

class IChunkGenerator
{
public:
    virtual ~IChunkGenerator() = default;
    virtual ChunkData Generate(const ChunkRegion& region) const = 0;
private:

};

#endif
