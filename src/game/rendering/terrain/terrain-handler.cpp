#include "terrain-handler.h"

#include "flat-chunk-generator.h"

TerrainHandler::TerrainHandler()
{
    chunkGenerator_ = std::make_unique<FlatChunkGenerator>();

    for (int x = -10; x <= 10; x++)
    {
        for (int z = -10; z <= 10; z++)
        {
            ChunkRegion region {
                { x, z },
                10.0f,
                16
            };
            ChunkData chunkData = chunkGenerator_->Generate(region);
            chunks_.push_back(chunkHandler_.UploadChunk(chunkData));
        }
    }
}

TerrainHandler::~TerrainHandler() = default;
