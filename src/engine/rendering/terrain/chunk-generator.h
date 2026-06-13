#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include <glm/glm.hpp>

#include "chunk-structs.h"


class IChunkGenerator
{
public:
    virtual ~IChunkGenerator() = default;
    virtual ChunkData Generate(const ChunkRegion& region) const = 0;
    virtual float HeightAt(float x, float z) const = 0;
    virtual glm::vec3 NormalAt(glm::vec3 position) const = 0;
private:

};

#endif
