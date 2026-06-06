#ifndef CHUNK_HANDLER_H
#define CHUNK_HANDLER_H

#include <memory>

class Mesh;
class ChunkData;

class ChunkHandler
{
public:
std::shared_ptr<Mesh> UploadChunk(const ChunkData& data);
private:
};

#endif
