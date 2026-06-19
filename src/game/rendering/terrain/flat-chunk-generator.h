#ifndef FLAT_CHUNK_GENERATOR
#define FLAT_CHUNK_GENERATOR

#include "../../../engine/rendering/terrain/chunk-generator.h"

class FlatChunkGenerator : public IChunkGenerator
{
public:
    ~FlatChunkGenerator() = default;
    ChunkData Generate(const ChunkRegion& region) const override;
    float HeightAt(float x, float z) const override;
    glm::vec3 NormalAt(glm::vec3 position) const override;
private:
};

#endif
