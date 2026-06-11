#include "flat-chunk-generator.h"

#include "fbm-noise.h"

ChunkData FlatChunkGenerator::Generate(const ChunkRegion& region) const
{
    ChunkData chunk;
    std::vector<Vertex> vertices;

    for (unsigned int i = 0; i <= region.resolution; i++)
    {
        for (unsigned int j = 0; j <= region.resolution; j++)
        {
            float u = (float)i / region.resolution;
            float v = (float)j / region.resolution;
            
            float vertX = region.coord.x * region.regionSize + u * region.regionSize;
            float vertZ = region.coord.y * region.regionSize + v * region.regionSize;

            float height = HeightAt(vertX, vertZ);

            vertices.push_back(
            {
                { 
                    vertX,
                    height,
                    vertZ
                },
                { 0, 0, 0},
                { u, v },
                { 0, 0, 0},
                { 0, 0, 0}
            });
        }
    }
    
    const unsigned int row = region.resolution + 1;
    const float d = region.regionSize / region.resolution;

    for (unsigned int i = 0; i <= region.resolution; i++)
    {
        for (unsigned int j = 0; j <= region.resolution; j++)
        {
            glm::vec3 normal;
            float u = (float)i / region.resolution;
            float v = (float)j / region.resolution;
            float vertX = region.coord.x * region.regionSize + u * region.regionSize;
            float vertZ = region.coord.y * region.regionSize + v * region.regionSize;

            if (i == 0 || i == region.resolution || j == 0 || j == region.resolution)
            {
                float dXp = HeightAt(vertX + d, vertZ);
                float dXn = HeightAt(vertX - d, vertZ);
                float dZp = HeightAt(vertX, vertZ + d);
                float dZn = HeightAt(vertX, vertZ - d);
                normal = glm::normalize(glm::vec3(dXn - dXp, 2.0f * d, dZn - dZp));
            }
            else
            { 
                float dXp = vertices.at((i+1) * row + j).Position.y;
                float dXn = vertices.at((i-1) * row + j).Position.y;
                float dZp = vertices.at(i * row + j + 1).Position.y;
                float dZn = vertices.at(i * row + j - 1).Position.y;
                normal = glm::normalize(glm::vec3(dXn - dXp, 2.0f * d, dZn - dZp));
            }
            vertices[i * row + j].Normal = normal;
        }
    }

    chunk.vertices = vertices;

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

    return chunk;
}

float FlatChunkGenerator::HeightAt(float x, float z) const
{
    return FBMNoise::GenNoise(octaves_, lacunarity_, gain_, glm::vec2(x, z) * baseFreq_) * heightScale_ + heightOffset_;
}
