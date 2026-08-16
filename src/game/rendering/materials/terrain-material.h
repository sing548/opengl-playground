#ifndef TERRAIN_MATERIAL_H
#define TERRAIN_MATERIAL_H

#include "../../../engine/models/asset-manager.h"
#include "../../../engine/rendering/render-list.h"
#include "../../../engine/rendering/materials/material.h"


class TerrainMaterial : public Material
{
public:
    explicit TerrainMaterial(std::unique_ptr<Shader> s, AssetManager& assMan, float bandMin, float bandMax);

    void ApplyFrame(const FrameGlobals& g) override;
    void ApplyInstance(const glm::mat4& model, const glm::vec4& tint) override;
private:
    GLuint grass_, rock_, snow_;
    GLuint gNorm_, rNorm_, sNorm_;
    float bandMin_, bandMax_;
};

#endif
