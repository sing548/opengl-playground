#include "fluid-renderer.h"

#include "fluid-data.h"
#include "../../gpu/shader.h"
#include "../../gpu/gpu-timer.h"
#include "../../gpu/gpu-buffer.h"
#include "../../gpu/compute-shader.h"
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

    // Run Compute Shader test here for lack of... caring enough to build a proper test env...

    constexpr uint32_t n = 1000;

    ComputeShader cs(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "test-iota.comp");

    GPUTimer gTimer;
    GPUBuffer gBuf(std::vector<uint32_t>(n, 0xFFFFFFFFu), GL_DYNAMIC_READ);

    gBuf.BindBase(0);
    cs.SetUInt("count", n);

    gTimer.StartTimer();
    cs.DispatchFor(n);
    gTimer.StopTimer();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    
    std::vector<uint32_t> res;
    gBuf.ReadData(res);

    for (uint32_t i = 0; i < res.size(); ++i)
    {
        if (res[i] != i)
            throw std::runtime_error("DOES NOT COMPUTE!");
    }

    std::cout << "Time for compute: " << gTimer.GetResultMS() << "ms\n";
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

    instanceCount_ = fluid_.GetParticleCount();

    if (fluid_.OnGPU())
    {
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        auto& gPart = fluid_.GetPositionsBuffer();
        glVertexArrayVertexBuffer(vao_, 1, gPart.Id(), 0, sizeof(glm::vec4));
    }
    else
    {
        glVertexArrayVertexBuffer(vao_, 1, instanceVBO_, 0, sizeof(glm::vec4));
        
        auto& particles = fluid_.GetParticles();
        std::vector<glm::vec4> particleData;

        for (auto& particle : particles)
        {
            glm::vec4 data = glm::vec4(particle.position, glm::length(particle.velocity));
            particleData.push_back(data);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
        glBufferData(GL_ARRAY_BUFFER, particleData.size()*sizeof(glm::vec4), particleData.data(), GL_DYNAMIC_DRAW);
    }
    
    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0, instanceCount_);
    glBindVertexArray(0);
}
