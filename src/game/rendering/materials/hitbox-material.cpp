#include "hitbox-material.h"

HitboxMaterial::HitboxMaterial(std::unique_ptr<Shader> shader) : Material(std::move(shader)) 
{
};

void HitboxMaterial::ApplyFrame(const FrameGlobals& globals)
{
    shader_->Use();
    shader_->SetMat4("projection", globals.projection);
    shader_->SetMat4("view", globals.view);
    
};

void HitboxMaterial::ApplyInstance(const glm::mat4& model, const glm::vec4& tint)
{
    shader_->SetMat4("model", model);
};
