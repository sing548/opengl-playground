#pragma once

#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <unordered_map>

#include "../../../engine/rendering/materials/material.h"
#include "../../../engine/rendering/terrain/chunk-structs.h"
#include "../../../engine/rendering/terrain/chunk-generator.h"

enum class WorldType
{
    BAKED,
    GENERATED
};

struct WorldInfo
{
    glm::vec3   Origin;
    float       Radius;
    WorldType   Type;
    std::string TilePath;
    std::string OriginTile;
};

struct World
{
    WorldInfo                                           info;
    std::unique_ptr<Material>                           material;
    std::unique_ptr<IChunkGenerator>                    generator;
    std::unordered_map<glm::ivec2, Chunk, IVec2Hash>    chunks;
};
