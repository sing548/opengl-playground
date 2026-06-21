#ifndef HITBOX_MATERIAL_H
#define HITBOX_MATERIAL_H

#include "../../../engine/rendering/render-list.h"
#include "../../../engine/rendering/materials/material.h"

class HitboxMaterial : public Material
{
public:
    explicit HitboxMaterial(std::unique_ptr<Shader> s);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
};

#endif
