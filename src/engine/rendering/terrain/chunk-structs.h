#pragma once

#include <memory>
#include <vector>

#include "../../models/mesh.h"

struct ChunkData {
    std::vector<Vertex>         vertices;
    std::vector<unsigned int>   indices;
};

struct ChunkRegion {
    glm::ivec2  coord;
    float       regionSize;
    int         resolution;
};

struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const
    {
        return (uint64_t)(uint32_t)v.x | ((uint64_t)(uint32_t)v.y << 32);
    }
};
struct Chunk {
    std::shared_ptr<Mesh> mesh;
    int lod;
};
