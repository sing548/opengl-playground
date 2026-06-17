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
        { -30, -30 },
        60.0f,
        16
    };

    std::vector<glm::vec3> offsets = GenerateOffsets(region, 18);
    instanceCount_ = (int) offsets.size();
    std::vector<glm::vec3> randoms = GenerateRandoms(instanceCount_);
    

    std::vector<unsigned int> indices;
    for (int i = 0; i + 2 < GRASS_BLADES_VERTICES; ++i)
        if (i % 2 == 0) { indices.push_back(i); indices.push_back(i+1); indices.push_back(i+2); }
        else            { indices.push_back(i+1); indices.push_back(i); indices.push_back(i+2); }
    indexCount_ = (int) indices.size();
    
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vertIDVBO_);
    glGenBuffers(1, &offsetVBO_);
    glGenBuffers(1, &randomsVBO_);
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

    glBindBuffer(GL_ARRAY_BUFFER, randomsVBO_);
    glBufferData(GL_ARRAY_BUFFER, randoms.size() * sizeof(glm::vec3), randoms.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(2, 1);

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

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);
    shader_->SetFloat("time", globals.time);

	shader_->SetVec3("dirLight.direction", globals.dirLight.direction);
	shader_->SetVec3("dirLight.ambient", globals.dirLight.ambient);   
	shader_->SetVec3("dirLight.diffuse", globals.dirLight.diffuse);

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

    shader_->SetFloat("fogStart", 100.0f);
    shader_->SetFloat("fogEnd", 500.0f);
    shader_->SetVec3("fogColor", glm::vec3(0.1f, 0.1f, 0.1f));

    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
    glBindVertexArray(0);
}

std::vector<glm::vec3> Grass::GenerateOffsets(const ChunkRegion& region, float density)
{
    const int bladesPerAxis = std::max(1, (int)std::round(region.regionSize * density));
    const float cellSize = region.regionSize / bladesPerAxis;

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

    return offsets;
}

std::vector<glm::vec3> Grass::GenerateRandoms(int count)
{
    std::vector<glm::vec3> randoms;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0.0, 1.0);

    for (int i = 0; i < count; i++)
        randoms.push_back({ (float)distrib(gen), (float)distrib(gen), (float)distrib(gen)});

    return randoms;
}
