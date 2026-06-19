#include "terrain-material.h"

#include "../terrain/terrain-config.h"
#include "../../../engine/helpers/file-helper.h"

TerrainMaterial::TerrainMaterial(Shader* shader, AssetManager& assMan) : Material(shader), assMan_(assMan)
{
    grass_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "rocky_terrain_02_1k" / "rocky_terrain_02_diff_1k.jpg", true);
    gNorm_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "rocky_terrain_02_1k" / "rocky_terrain_02_nor_gl_1k.jpg");
    rock_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "seaside_rock_1k" / "seaside_rock_diff_1k.jpg", true);
    rNorm_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "seaside_rock_1k" / "seaside_rock_nor_gl_1k.jpg");
    snow_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "snow_02_1k"/ "snow_02_diff_1k.jpg", true);
    gNorm_ = assMan_.LoadTexture(std::filesystem::path(
                    FileHelper::GetAssetsDir()) / "textures" / "snow_02_1k" / "snow_02_nor_gl_1k.jpg");

    
}

void TerrainMaterial::ApplyFrame(const FrameGlobals& globals)
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
        shader_->SetVec3(str + ".specular", light.specular);
        shader_->SetFloat(str + ".constant", light.constant);
        shader_->SetFloat(str + ".linear", light.linear);
        shader_->SetFloat(str + ".quadratic", light.quadratic);
    }
    shader_->SetInt("numPointLights", static_cast<int>(n));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grass_);
    shader_->SetInt("grassTex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rock_);
    shader_->SetInt("rockTex", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, snow_);
    shader_->SetInt("snowTex", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gNorm_);
    shader_->SetInt("grassNormal", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, rNorm_);
    shader_->SetInt("rockNormal", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, sNorm_);
    shader_->SetInt("snowNormal", 5);

    shader_->SetFloat("snowStart", TerrainConfig::SnowStart);
    shader_->SetFloat("snowEnd", TerrainConfig::SnowEnd);
    shader_->SetFloat("rockStart", TerrainConfig::RockStart);
    shader_->SetFloat("rockEnd", TerrainConfig::RockEnd);

    shader_->SetFloat("fogStart", TerrainConfig::FogStart);
    shader_->SetFloat("fogEnd", TerrainConfig::FogEnd);
    shader_->SetVec3("fogColor", glm::vec3(TerrainConfig::FogColor, TerrainConfig::FogColor, TerrainConfig::FogColor));

    shader_->SetFloat("texScale", 0.2);
}

void TerrainMaterial::ApplyInstance(const glm::mat4& model, const glm::vec4& tint)
{
    shader_->SetMat4("model", model);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    shader_->SetMat3("normalMatrix", normalMatrix);
}
