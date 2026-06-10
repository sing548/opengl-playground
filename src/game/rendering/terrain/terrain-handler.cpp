#include "terrain-handler.h"

#include "flat-chunk-generator.h"

TerrainHandler::TerrainHandler()
{
    chunkGenerator_ = std::make_unique<FlatChunkGenerator>();

    for (int x = -3; x <= 3; x++)
    {
        for (int z = -3; z <= 3; z++)
        {
            ChunkRegion region {
                { x, z },
                30.0f,
                32
            };
            ChunkData chunkData = chunkGenerator_->Generate(region);
            chunks_.push_back(chunkHandler_.UploadChunk(chunkData));
        }
    }
}

TerrainHandler::~TerrainHandler() = default;
