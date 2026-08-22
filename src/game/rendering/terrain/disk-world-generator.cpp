#include "disk-world-generator.h"

#include "../../../engine/helpers/file-helper.h"

#include "baked-map-generator.h"

DiskWorldGenerator::DiskWorldGenerator(WorldInfo wi) 
    : radius_(wi.Radius), edge_(wi.Edge), thickness_(wi.Thickness), worldGenerator_(std::make_unique<BakedMapGenerator>(wi))
{
}

ChunkData DiskWorldGenerator::Generate(const ChunkRegion& region) const
{
    ChunkData d = worldGenerator_->Generate(region);

    const float inner = radius_ - edge_;
    constexpr float eps = 1.6f;

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

    for (auto& v : d.vertices)
    {
        const float r = std::sqrt(v.Position.x * v.Position.x + v.Position.z * v.Position.z);
        v.Position.y = HeightAt(v.Position.x, v.Position.z);

        if (r > inner - eps)
            v.Normal = NormalAt(v.Position);
    }

    return d;
}

float DiskWorldGenerator::HeightAt(float x, float z) const
{
    const float relief = worldGenerator_->HeightAt(x,z) - worldGenerator_->MinHeight();
    return InnerHeight(x, z, relief + thickness_);
}

glm::vec3 DiskWorldGenerator::NormalAt(glm::vec3 pos) const
{
    constexpr float epsilon = 1.6f;

    float dXp = HeightAt(pos.x + epsilon, pos.z);
    float dXn = HeightAt(pos.x - epsilon, pos.z);
    float dZp = HeightAt(pos.x, pos.z + epsilon);
    float dZn = HeightAt(pos.x, pos.z - epsilon);

    return glm::normalize(glm::vec3(dXn - dXp, 2.0f * epsilon, dZn - dZp));
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