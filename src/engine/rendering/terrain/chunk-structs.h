#pragma once

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
