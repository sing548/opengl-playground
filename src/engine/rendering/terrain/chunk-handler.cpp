#include "chunk-handler.h"

#include "chunk-structs.h"
#include "../../models/mesh.h"

std::shared_ptr<Mesh> ChunkHandler::UploadChunk(ChunkData&& data)
{
    std::vector<Texture> emptyTextures;
    if (data.indices.empty())
        return nullptr;
    return std::make_shared<Mesh>(std::move(data.vertices), std::move(data.indices), std::move(emptyTextures));
}
