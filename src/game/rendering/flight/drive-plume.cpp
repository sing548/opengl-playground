#include "drive-plume.h"

#include <vector>
#include <filesystem>

#include "../../../engine/shaders/shader.h"
#include "../../../engine/helpers/file-helper.h"
#include "../../../engine/rendering/render-list.h"

DrivePlume::DrivePlume(GameWorld& world) : world_(world)
{
    const int DRIVE_PLUME_VERTICES = 4;

    std::string vert = (std::filesystem::path(FileHelper::GetShaderDir()) / "drive-plume.vert").string();
    std::string frag = (std::filesystem::path(FileHelper::GetShaderDir()) / "drive-plume.frag").string();
    shader_ = std::make_unique<Shader>(vert.c_str(), frag.c_str());

    std::vector<float> vertIDs;

    for (int i = 0; i < DRIVE_PLUME_VERTICES; i++)
        vertIDs.push_back((float)i);
    
    std::vector<unsigned int> indices = { 0,1,2, 2,1,3 };
    indexCount_ = (int)indices.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vertIDVBO_);
    glGenBuffers(1, &instanceVBO_);
    glGenBuffers(1, &indexBufferEBO_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vertIDVBO_);
    glBufferData(GL_ARRAY_BUFFER, vertIDs.size() * sizeof(float), vertIDs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

DrivePlume::~DrivePlume()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vertIDVBO_);
    glDeleteBuffers(1, &instanceVBO_);
    glDeleteBuffers(1, &indexBufferEBO_);
}

void DrivePlume::Render(const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings)
{
    shader_->Use();

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);

    int n = 0;

    std::vector<glm::vec4> nozzleData;

    for (auto& [id, _] : world_.GetPlayerData())
    {
        auto& model = world_.GetScene().GetModelByReference(id);

        glm::vec3 nozzle = model.GetPosition() - model.GetForward() * 0.5f;
        float velo = glm::length(model.GetVelocity());
        nozzleData.push_back(glm::vec4(nozzle, velo));
        n++;
    }

    for (auto& [id, _] : world_.GetNpcData())
    {
        auto& model = world_.GetScene().GetModelByReference(id);
        const auto& interpolatedPi = model.GetInterpolatedInfo();

        glm::vec3 nozzle = interpolatedPi.position_ - (interpolatedPi.rotation_ * model.GetBaseOrientation()) * 0.5f;
        float velo = glm::length(model.GetVelocity());
        nozzleData.push_back(glm::vec4(nozzle, velo));
        n++;
    }

    instanceCount_ = n;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER, nozzleData.size()*sizeof(glm::vec4), nozzleData.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
    glBindVertexArray(0);
}
