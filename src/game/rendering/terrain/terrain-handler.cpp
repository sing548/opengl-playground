#include "terrain-handler.h"

#include "flat-chunk-generator.h"
#include "../../../engine/rendering/render-list.h"

TerrainHandler::TerrainHandler()
{
    chunkGenerator_ = std::make_unique<FlatChunkGenerator>();
}

void TerrainHandler::HandleChunksForArea(const glm::ivec2& area)
{
    for (int i =  area.x - TerrainConfig::RenderArea; i <= area.x + TerrainConfig::RenderArea; i++)
    {
        for (int j = area.y - TerrainConfig::RenderArea; j <= area.y + TerrainConfig::RenderArea; j++)
        {
            auto it = chunks_.find({ i, j });
            int dist = std::max(std::abs(i - area.x), std::abs(j - area.y));
            bool lowLod = dist > TerrainConfig::LowLoDArea;
            bool deleted = false;

            if (it != chunks_.end() && (
                lowLod && it->second.lod != TerrainConfig::LowLodRegionResolution ||
                    !lowLod && it->second.lod != TerrainConfig::RegionResolution
                ))
            {
                chunks_.erase({ i, j });
                deleted = true;
            }

            if (deleted || it == chunks_.end())
            {
                ChunkRegion region {
                    { i, j },
                    TerrainConfig::RegionSize,
                    lowLod ?  TerrainConfig::LowLodRegionResolution : TerrainConfig::RegionResolution
                };
                ChunkData data = chunkGenerator_->Generate(region);
                Chunk chunk;
                chunk.mesh = chunkHandler_.UploadChunk(data);
                chunk.lod = lowLod ?  TerrainConfig::LowLodRegionResolution : TerrainConfig::RegionResolution;
                chunks_.emplace(glm::ivec2(i, j), chunk);
            }
        }
    }

    for (auto it = chunks_.begin(); it != chunks_.end();)
    {
        glm::ivec2 chunk = it->first;

        if (chunk.x < area.x - TerrainConfig::RenderArea - TerrainConfig::Hysteresis ||
            chunk.x > area.x + TerrainConfig::RenderArea + TerrainConfig::Hysteresis ||
            chunk.y < area.y - TerrainConfig::RenderArea - TerrainConfig::Hysteresis ||
            chunk.y > area.y + TerrainConfig::RenderArea + TerrainConfig::Hysteresis )
            it = chunks_.erase(it);
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

    float bottom = pos.y - radius;
    // ToDo: Move HeightOffset to TerrainConfig struct and out of ChunkGenerator
    if (bottom > 18.0f /*heightOffset - will be moved*/)
    {
        return col;
    }

    float height = chunkGenerator_->HeightAt(pos.x, pos.z);

    col.collided = bottom < height;
    col.penetration = height - bottom;
    
    if (col.collided)
        col.normal = chunkGenerator_->NormalAt(pos);
    
    return col;
}
