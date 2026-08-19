#include "chunk-handler.h"

#include "chunk-structs.h"
#include "../../models/mesh.h"

std::shared_ptr<Mesh> ChunkHandler::UploadChunk(const ChunkData& data)
{
    std::vector<Texture> emptyTextures;
    return std::make_shared<Mesh>(std::move(data.vertices), std::move(data.indices), std::move(emptyTextures));
}
