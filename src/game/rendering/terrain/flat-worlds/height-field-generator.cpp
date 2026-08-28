#include "height-field-generator.h"

ChunkData HeightFieldGenerator::Generate(const ChunkRegion& region) const
{   
    ChunkData chunk;
    std::vector<Vertex> vertices;
    vertices.reserve((region.resolution + 1) * (region.resolution + 1));

    // Calculate height for each point of resolution by interpolating height from .png
    for (auto i = 0; i <= region.resolution; i++)
    {
        for (auto j = 0; j <= region.resolution; j++)
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

    // Calculate Normals
    for (auto i = 0; i <= region.resolution; i++)
    {
        for (auto j = 0; j <= region.resolution; j++)
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

    chunk.vertices = std::move(vertices);


    // Calculate indices
    chunk.indices.reserve(region.resolution * region.resolution * 6);

    for (auto i = 0; i < region.resolution; i++)
    {
        for (auto j = 0; j < region.resolution; j++)
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

glm::vec3 HeightFieldGenerator::NormalAt(glm::vec3 pos) const 
{
    float epsilon = 0.1f;
    float dXp = HeightAt(pos.x + epsilon, pos.z);
    float dXn = HeightAt(pos.x - epsilon, pos.z);
    float dZp = HeightAt(pos.x, pos.z + epsilon);
    float dZn = HeightAt(pos.x, pos.z - epsilon);

    return glm::normalize(glm::vec3(dXn - dXp, 2.0f * epsilon, dZn - dZp));
}
