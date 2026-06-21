#ifndef MODEL_MATERIAL_H
#define MODEL_MATERIAL_H

#include <glm/ext/matrix_clip_space.hpp>

#include "../../../engine/rendering/render-list.h"
#include "../../../engine/rendering/materials/material.h"

class ModelMaterial : public Material
{
public:
    explicit ModelMaterial(std::unique_ptr<Shader> s);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
};

#endif
