#include "hitbox-material.h"

HitboxMaterial::HitboxMaterial(Shader* shader) : shader_(shader) 
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

Shader& HitboxMaterial::GetShader()
{
    return *shader_;
};
