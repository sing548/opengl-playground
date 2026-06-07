#ifndef TERRAIN_MATERIAL_H
#define TERRAIN_MATERIAL_H

#include "../../../engine/rendering/render-list.h"
#include "../../../engine/rendering/materials/material.h"

class TerrainMaterial : public Material
{
public:
    explicit TerrainMaterial(Shader* s);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
};

#endif
