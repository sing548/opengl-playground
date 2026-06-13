#include "terrain-handler.h"

#include "flat-chunk-generator.h"
#include "../../../engine/rendering/render-list.h"

TerrainHandler::TerrainHandler()
{
    chunkGenerator_ = std::make_unique<FlatChunkGenerator>();
}

void TerrainHandler::HandleChunksForArea(const glm::ivec2& area)
{
    int renderedArea = 5;

    for (int i =  area.x - renderedArea; i <= area.x + renderedArea; i++)
    {
        for (int j = area.y - renderedArea; j <= area.y + renderedArea; j++)
        {
            if (chunks_.find({ i, j}) == chunks_.end())
            {
                ChunkRegion region {
                    { i, j },
                    regionSize_,
                    regionResolution_
                };
                ChunkData data = chunkGenerator_->Generate(region);
                chunks_.emplace(glm::ivec2(i, j), chunkHandler_.UploadChunk(data));
            }
        }
    }

    for (auto it = chunks_.begin(); it != chunks_.end();)
    {
        glm::ivec2 chunk = it->first;

        if (chunk.x < area.x - renderedArea - hysteresis_ || chunk.x > area.x + renderedArea + hysteresis_  ||
            chunk.y < area.y - renderedArea - hysteresis_  || chunk.y > area.y + renderedArea + hysteresis_ )
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
