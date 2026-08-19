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

    for (auto& v : d.vertices)
    {
        const float r = std::sqrt(v.Position.x * v.Position.x + v.Position.z * v.Position.z);
        v.Position.y = InnerHeight(v.Position.x, v.Position.z, v.Position.y);

        if (r > inner - eps)
            v.Normal = NormalAt(v.Position);
    }

    return d;
}

float DiskWorldGenerator::HeightAt(float x, float z) const
{
    return InnerHeight(x, z, worldGenerator_->HeightAt(x, z));
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
    const float floory = worldGenerator_->MinHeight() - thickness_;
    const float r      = std::sqrt(x * x + z * z);

    float t = glm::clamp((r - inner) / std::max(edge_, 0.001f), 0.0f, 1.0f);
    t = t * t * (3.0f - 2.0f * t);

    return glm::mix(h, floory, t);
}