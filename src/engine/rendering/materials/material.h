#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/ext/matrix_clip_space.hpp>

#include "../../rendering/render-list.h"
#include "../../shaders/shader.h"

class Material
{
public:
    virtual ~Material() = default;
    virtual void ApplyFrame(const FrameGlobals&) = 0;
    virtual void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) = 0;
    virtual Shader& GetShader() {{ return *shader_;}};
protected:
    explicit Material(Shader* shader) : shader_(shader) { }
    Shader* shader_ = nullptr;
};

#endif
