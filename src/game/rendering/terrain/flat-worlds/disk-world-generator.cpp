#include "disk-world-generator.h"

#include "../../../../engine/helpers/file-helper.h"

#include "baked-map-generator.h"
#include "flat-chunk-generator.h"

DiskWorldGenerator::DiskWorldGenerator(WorldInfo wi) 
    : radius_(wi.Radius), edge_(wi.Edge), thickness_(wi.Thickness)
{
    if (wi.Type == WorldType::BAKED)
        worldGenerator_ = std::make_unique<BakedMapGenerator>(wi);
    else
        worldGenerator_ =  std::make_unique<FlatChunkGenerator>();
}

ChunkData DiskWorldGenerator::Generate(const ChunkRegion& region) const
{
    ChunkData d = HeightFieldGenerator::Generate(region);

    std::vector<unsigned int> kept;
    kept.reserve(d.indices.size());

    for (size_t i = 0; i +2 < d.indices.size(); i+= 3)
    {
        bool anyInsideDisk = false;

        for (int j = 0; j < 3; ++j)
        {
            const auto& pos = d.vertices[d.indices[i + j]].Position;
            
            if (pos.x * pos.x + pos.z * pos.z <= radius_ * radius_)
            {
                anyInsideDisk = true;
                break;
            }
        }

        if (anyInsideDisk)
        {
            kept.push_back(d.indices[i]);
            kept.push_back(d.indices[i + 1]);
            kept.push_back(d.indices[i + 2]);
        }
    }

    d.indices = std::move(kept);

    return d;
}

float DiskWorldGenerator::HeightAt(float x, float z) const
{
    const float relief = worldGenerator_->HeightAt(x,z) - worldGenerator_->MinHeight();
    return InnerHeight(x, z, relief + thickness_);
}

float DiskWorldGenerator::InnerHeight(float x, float z, float h) const
{
    const float inner  = radius_ - edge_;
    const float floory = 0.0f;
    const float r      = std::sqrt(x * x + z * z);

    float t = glm::clamp((r - inner) / std::max(edge_, 0.001f), 0.0f, 1.0f);
    t = t * t * (3.0f - 2.0f * t);

    return glm::mix(h, floory, t);
}