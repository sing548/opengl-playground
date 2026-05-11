#ifndef HITBOX_MATERIAL_H
#define HITBOX_MATERIAL_H

#include "material.h"
#include "../../rendering/render-list.h"

class HitboxMaterial : public Material
{
public:
    explicit HitboxMaterial(Shader* s);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
    Shader& GetShader() override;
private:
    Shader* shader_;
};

#endif
