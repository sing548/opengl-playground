#ifndef MODEL_MATERIAL_H
#define MODEL_MATERIAL_H

#include <glm/ext/matrix_clip_space.hpp>

#include "material.h"
#include "../../rendering/render-list.h"

class ModelMaterial : public Material
{
public:
    explicit ModelMaterial(Shader* s);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
    Shader& GetShader() override;
private:
    Shader* shader_;
};

#endif
