#ifndef TERRAIN_HANDLER_H
#define TERRAIN_HANDLER_H

#include <memory>
#include <vector>

#include "../../../engine/rendering/terrain/chunk-handler.h"

class IChunkGenerator;

class TerrainHandler
{
public:
    TerrainHandler();
    ~TerrainHandler();
    const std::vector<std::shared_ptr<Mesh>>& GetChunks() const { return chunks_; }
private:
    ChunkHandler chunkHandler_;
    std::unique_ptr<IChunkGenerator> chunkGenerator_;

    std::vector<std::shared_ptr<Mesh>> chunks_;
};

#endif
