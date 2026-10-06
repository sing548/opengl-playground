#include "fluid-renderer.h"

#include "fluid-data.h"
#include "../../shaders/shader.h"
#include "../../helpers/file-helper.h"
#include "../../rendering/render-list.h"

FluidRenderer::FluidRenderer(const FluidData& fluid) : fluid_(fluid)
{
    const int FLUID_VERTICES = 4;

    std::string vert = (std::filesystem::path(FileHelper::GetShaderDir()) / "fluid.vert").string();
    std::string frag = (std::filesystem::path(FileHelper::GetShaderDir()) / "fluid.frag").string();
    shader_ = std::make_unique<Shader>(vert.c_str(), frag.c_str());

    std::vector<float> vertIDs;

    for (int i = 0; i < FLUID_VERTICES; ++i)
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

FluidRenderer::~FluidRenderer()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vertIDVBO_);
    glDeleteBuffers(1, &instanceVBO_);
    glDeleteBuffers(1, &indexBufferEBO_);
}

void FluidRenderer::Render(const FrameGlobals& globals)
{
    shader_->Use();

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);

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
        shader_->SetFloat(str + ".constant", light.constant);
        shader_->SetFloat(str + ".linear", light.linear);
        shader_->SetFloat(str + ".quadratic", light.quadratic);
    }
    shader_->SetInt("numPointLights", static_cast<int>(n));

    shader_->SetFloat("particleSize", fluid_.GetParticleSize());

    std::vector<glm::vec4> particleData;
    int nInst = 0;

    auto& particles = fluid_.GetParticles();

    for (auto& particle : particles)
    {
        glm::vec4 data = glm::vec4(particle.position, glm::length(particle.velocity));
        particleData.push_back(data);
        ++nInst;
    }

    instanceCount_ = nInst;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER, particleData.size()*sizeof(glm::vec4), particleData.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
    glBindVertexArray(0);
}
