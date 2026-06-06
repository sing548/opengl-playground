#include "chunk-handler.h"

#include "chunk-structs.h"
#include "../../models/mesh.h"

std::shared_ptr<Mesh> ChunkHandler::UploadChunk(const ChunkData& data)
{
    std::vector<Texture> emptyTextures;
    return std::make_shared<Mesh>(data.vertices, data.indices, emptyTextures);
}
