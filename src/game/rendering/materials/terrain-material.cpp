#include "terrain-material.h"

TerrainMaterial::TerrainMaterial(Shader* shader) : Material(shader)
{
}

void TerrainMaterial::ApplyFrame(const FrameGlobals& globals)
{
    shader_->Use();

    glm::vec3 dirLight(-1.0f, 0.0f, 0.0f);

    shader_->SetVec3("viewPos", globals.cameraPos);
	shader_->SetMat4("projection", globals.projection);
	shader_->SetMat4("view", globals.view);

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
}

void TerrainMaterial::ApplyInstance(const glm::mat4& model, const glm::vec4& tint)
{
    shader_->SetMat4("model", model);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    shader_->SetMat3("normalMatrix", normalMatrix);
}
