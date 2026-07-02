#ifndef DRIVE_PLUME_H
#define DRIVE_PLUME_H

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../../../engine/rendering/i-scene-rendererable.h"

#include "../../game-world/game-world.h"

struct ChunkRegion;
struct FrameGlobals;

class Shader;

class DrivePlume : public ISceneRenderable
{
public:
    DrivePlume(GameWorld& world);
    ~DrivePlume();

    void Render(const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings) override;
    RenderPass GetRenderPass() override { return RenderPass::Emissive; };
    int GetOrder() override { return 20; };
private:
    float height_ = -1.0f;
    std::unique_ptr<Shader> shader_;
    unsigned int vertIDVBO_, instanceVBO_, indexBufferEBO_, vao_;
    int indexCount_, instanceCount_;

    GameWorld& world_;
};

#endif
