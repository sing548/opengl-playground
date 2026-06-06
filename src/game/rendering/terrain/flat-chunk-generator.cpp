#include "flat-chunk-generator.h"

ChunkData FlatChunkGenerator::Generate(const ChunkRegion& region) const
{
    const float height = 0;

    ChunkData chunk;
    std::vector<Vertex> vertices;

    for (unsigned int i = 0; i <= region.resolution; i++)
    {
        for (unsigned int j = 0; j <= region.resolution; j++)
        {
            float u = (float)i / region.resolution;
            float v = (float)j / region.resolution;
            
            /*Vertex vert;
            vert.Position = 
            { 
                region.coord.x * region.worldSize + u * region.worldSize,
                height,
                region.coord.y * region.worldSize + v * region.worldSize
            };
            vert.Normal = { 0, 1, 0 };
            vert.TexCoords = { u, v };
            vert.Tangent =  { 0, 0, 0 };
            vert.Bitangent = { 0, 0, 0 };
            vertices.push_back(vert);*/

            vertices.push_back(
            {
                { 
                    region.coord.x * region.worldSize + u * region.worldSize,
                    height,
                    region.coord.y * region.worldSize + v * region.worldSize
                },
                { 0, 1, 0},
                { u, v },
                { 0, 0, 0},
                { 0, 0, 0}
            });
        }
    }

    const unsigned int row = region.resolution + 1;
    chunk.indices.reserve(region.resolution * region.resolution * 6);

    for (unsigned int i = 0; i < region.resolution; i++)
    {
        for (unsigned int j = 0; j < region.resolution; j++)
        {
            unsigned int v00 = i * row + j;
            unsigned int v01 = i * row + j + 1;
            unsigned int v10 = (i + 1) * row + j;
            unsigned int v11 = (i + 1) * row + j + 1;

            chunk.indices.push_back(v00);
            chunk.indices.push_back(v10);
            chunk.indices.push_back(v11);

            chunk.indices.push_back(v00);
            chunk.indices.push_back(v11);
            chunk.indices.push_back(v01);
        }
    }

    chunk.vertices = vertices;

    return chunk;
}