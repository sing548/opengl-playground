#include "grass.h"

#include <chrono>
#include <random>
#include <vector>
#include <filesystem>

#include "../../../engine/shaders/shader.h"
#include "../../../engine/helpers/file-helper.h"
#include "../../../engine/rendering/render-list.h"

#include "../../../engine/rendering/terrain/chunk-structs.h"

Grass::Grass()
{
    const int GRASS_BLADES_VERTICES = 15;

    std::string vert = (std::filesystem::path(FileHelper::GetShaderDir()) / "grass.vert").string();
    std::string frag = (std::filesystem::path(FileHelper::GetShaderDir()) / "grass.frag").string(); 
    shader_ = std::make_unique<Shader>(vert.c_str(), frag.c_str());

    std::vector<float> vertIDs;

    for (int i = 0; i < GRASS_BLADES_VERTICES; i++)
        vertIDs.push_back((float)i);

    
    

    ChunkRegion region {
        { -40, -40 },
        80.0f,
        16
    };

    std::vector<glm::vec3> offsets = GenerateOffsets(region, 20);
    
    instanceCount_ = (int) offsets.size();

    std::vector<unsigned int> indices;
    for (int i = 0; i + 2 < GRASS_BLADES_VERTICES; ++i)
        if (i % 2 == 0) { indices.push_back(i); indices.push_back(i+1); indices.push_back(i+2); }
        else            { indices.push_back(i+1); indices.push_back(i); indices.push_back(i+2); }
    indexCount_ = (int) indices.size();
    
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vertIDVBO_);
    glGenBuffers(1, &offsetVBO_);
    glGenBuffers(1, &indexBufferEBO_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vertIDVBO_);
    glBufferData(GL_ARRAY_BUFFER, vertIDs.size() * sizeof(float), vertIDs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, offsetVBO_);
    glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(glm::vec3), offsets.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

Grass::~Grass()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vertIDVBO_);
    glDeleteBuffers(1, &offsetVBO_);
    glDeleteBuffers(1, &indexBufferEBO_);
}

void Grass::Render(const FrameGlobals& globals)
{
    shader_->Use();

    glm::vec3 dirLight(-1.0f, -1.0f, 0.0f);

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);
    shader_->SetFloat("time", globals.time);

	shader_->SetVec3("dirLight.direction", dirLight);
	shader_->SetVec3("dirLight.ambient", glm::vec3(0.2f, 0.22f, 0.25f));   
	shader_->SetVec3("dirLight.diffuse", glm::vec3(0.4f, 0.42f, 0.45f));

    constexpr size_t kMaxLights = 128;
    const size_t n = std::min(globals.pointLights.size(), kMaxLights);

    for (size_t i = 0; i < n; i++)
    {
        const auto& light = globals.pointLights[i];
        const std::string str = "pointLights[" + std::to_string(i) + "]";

        shader_->SetVec3(str + ".position", light.position);
        shader_->SetVec3(str + ".ambient", light.ambient);
        shader_->SetVec3(str + ".diffuse", light.diffuse);
        shader_->SetVec3(str + ".specular", light.specular);
        shader_->SetFloat(str + ".constant", light.constant);
        shader_->SetFloat(str + ".linear", light.linear);
        shader_->SetFloat(str + ".quadratic", light.quadratic);
    }
    shader_->SetInt("numPointLights", static_cast<int>(n));

    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
    glBindVertexArray(0);
}

std::vector<glm::vec3> Grass::GenerateOffsets(const ChunkRegion& region, float density)
{
    const int NUM_GRASS_X = 1024, NUM_GRASS_Z = 1024;
    const float GRASS_SPACING = 0.08f;


    const int bladesPerAxis = std::max(1, (int)std::round(region.worldSize * density));
    const float cellSize = region.worldSize / bladesPerAxis;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0.0, 1.0);

    std::vector<glm::vec3> offsets;
    offsets.reserve(bladesPerAxis * bladesPerAxis);



    for (int x = 0; x < bladesPerAxis; ++x)
    {
        for (int z = 0; z < bladesPerAxis; ++z)
        {
            float xJitter = (float)distrib(gen);
            float zJitter = (float)distrib(gen);

            float xOffset = region.coord.x + (x + xJitter) * cellSize;
            float zOffset = region.coord.y + (z + zJitter) * cellSize;
            offsets.push_back({ xOffset, height_, zOffset });
        }
    }

    /*for (int x = 0; x < NUM_GRASS_X; ++x)
        for (int z = 0; z < NUM_GRASS_Z; ++z)
        {
            float xOffset = (float)distrib(gen);
            float zOffset = (float)distrib(gen);
            offsets.push_back({ x * GRASS_SPACING + xOffset, height_, z * GRASS_SPACING + zOffset});
        }
    */

    return offsets;
}
