#include "terrain-handler.h"

TerrainHandler::TerrainHandler(std::vector<World> worlds) : worlds_(std::move(worlds)) {}

TerrainHandler::~TerrainHandler() = default;

void TerrainHandler::UpdateStreaming(const glm::vec3& observerPos)
{
    for (auto& world : worlds_)
    {
        const glm::vec3 local = world.info.ToLocalCoords(observerPos);

        const glm::ivec2 area {
            (int)std::floor(local.x / TerrainConfig::RegionSize),
            (int)std::floor(local.z / TerrainConfig::RegionSize)
        };

        constexpr float renderRange = TerrainConfig::RenderArea * TerrainConfig::RegionSize;
        constexpr float exitMargin  = TerrainConfig::Hysteresis * TerrainConfig::RegionSize;

        const float margin = world.lastCheckInRange ? renderRange + exitMargin : renderRange;

        const float d2 = local.x * local.x + local.z * local.z;
        const float radial = world.info.Radius + margin;

        const float top     = world.generator->MaxHeight();
        const float bottom  = world.generator->MinHeight() - world.info.Thickness;

        const float enter = world.info.Radius + renderRange;
        const float exit = enter + exitMargin;
        
        const float normalDist = local.y > top    ? local.y - top
                               : local.y < bottom ? bottom - local.y
                               : 0.0f;

        const bool inRange = d2 <= radial * radial
                          && normalDist <= margin;

        const bool changed = (area != world.lastArea) || (inRange != world.lastCheckInRange);
        world.lastArea = area;
        world.lastCheckInRange = inRange;

        if (!changed) continue;
        
        if (inRange) RefreshChunks(world, area);
        CullChunks(world, area);
    }
}

void TerrainHandler::RefreshChunks(World& world, const glm::ivec2 area)
{
    for (int i = area.x - TerrainConfig::RenderArea; i <= area.x + TerrainConfig::RenderArea; i++)
    {
        for (int j = area.y - TerrainConfig::RenderArea; j <= area.y + TerrainConfig::RenderArea; j++)
        {
            if (!world.info.ChunkOnDisk({ i, j }, TerrainConfig::RegionSize)) continue;

            const int dist    = std::max(std::abs(i - area.x), std::abs(j - area.y));
            const bool lowLod = dist > TerrainConfig::LowLoDArea;
            const int lod     = lowLod ? TerrainConfig::LowLodRegionResolution : TerrainConfig::RegionResolution;

            auto it = world.chunks.find({ i, j});

            if (it != world.chunks.end())
            {
                // Only re-generate higher quality. Don't regenerate lower quality - causes stuttering in frametimws 
                // rather than just a little higher draw count
                if (lowLod || it->second.lod == TerrainConfig::RegionResolution) continue;

                world.chunks.erase(it);
            }

            ChunkRegion region {
                { i, j },
                TerrainConfig::RegionSize,
                lod
            };

            Chunk chunk;
            chunk.mesh = chunkHandler_.UploadChunk(world.generator->Generate(region));
            chunk.lod = lod;
            world.chunks.emplace(glm::ivec2(i, j), chunk);
        }
    }
}

void TerrainHandler::CullChunks(World& world, const glm::ivec2& area)
{
    constexpr int limit = TerrainConfig::RenderArea + TerrainConfig::Hysteresis;

    for (auto it = world.chunks.begin(); it != world.chunks.end();)
    {
        const glm::ivec2 chunk = it->first;

        const bool chunkOutsideDisk = !world.info.ChunkOnDisk(chunk, TerrainConfig::RegionSize);
        const bool outOfRange       = std::abs(chunk.x - area.x) > limit || std::abs(chunk.y - area.y) > limit;

        if (chunkOutsideDisk || outOfRange)
            it = world.chunks.erase(it);
        else
            ++it;
    }
}

TerrainHandler::TerrainCollision TerrainHandler::CheckCollision(glm::vec3 pos, float radius)
{
    TerrainCollision col
    {
        false,
        0.0f,
        { 0.0f, 0.0f, 0.0f}
    };

    for (auto& world : worlds_)
    {
        const glm::vec3 local = world.info.ToLocalCoords(pos);

        if (local.y < 0.0f)
            continue;

        if (local.x * local.x + local.z * local.z >  world.info.Radius * world.info.Radius)
            continue;

        const float bottom = local.y - radius;
        if (bottom > world.generator->MaxHeight())
            continue;

        const float height = world.generator->HeightAt(local.x, local.z);
        if (bottom >= height)
            continue;

        col.collided = true;
        col.penetration = height - bottom;
        col.normal = world.info.Orientation * world.generator->NormalAt(local);
        break;
    }

    return col;
}

std::vector<DrawCommand> TerrainHandler::BuildDrawCommands(RenderPass rp)
{
    std::vector<DrawCommand> dcs;

    size_t reserveSpace = 0;
    for (auto& world : worlds_) reserveSpace += world.chunks.size();

    dcs.reserve(reserveSpace);

    for (auto& world : worlds_)
    {

        const glm::mat4 transform = world.info.Transform();
    
        for (auto& [iv, mp] : world.chunks)
        {
            if (!mp.mesh) continue;
            
            DrawCommand dc;
            dc.mesh = mp.mesh.get();
            dc.material = world.material.get();
            dc.transform = transform;
            dc.renderPass = rp;
            dcs.push_back(dc);
        }
    }

    return dcs;
}
