#pragma once

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../../rendering/i-scene-rendererable.h"

struct FrameGlobals;

class Shader;
class FluidData;

class FluidRenderer : public ISceneRenderable
{
public:
    FluidRenderer(const FluidData& fluid);
    ~FluidRenderer();

    void Render(const FrameGlobals& globals) override;
    RenderPass GetRenderPass() override { return RenderPass::Opaque; };
    int GetOrder() override { return 5; };
private:
    const FluidData& fluid_;
    std::unique_ptr<Shader> shader_;
    unsigned int vertIDVBO_, instanceVBO_, indexBufferEBO_, vao_;
    int indexCount_, instanceCount_;
};
