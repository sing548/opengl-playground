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
