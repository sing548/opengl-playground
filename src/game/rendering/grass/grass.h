#ifndef GRASS_H
#define GRASS_H

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../../../engine/rendering/i-scene-rendererable.h"

struct ChunkRegion;
struct FrameGlobals;

class Shader;

class Grass : public ISceneRenderable
{
public:
    Grass();
    ~Grass();

    void Render(const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings) override;
    RenderPass GetRenderPass() override { return RenderPass::Opaque; };
private:
    float height_ = -1.0f;
    std::unique_ptr<Shader> shader_;
    unsigned int vertIDVBO_, offsetVBO_, randomsVBO_, indexBufferEBO_, vao_;
    int indexCount_, instanceCount_;

    std::vector<glm::vec3> GenerateOffsets(const ChunkRegion& region, float density);
    std::vector<glm::vec3> GenerateRandoms(int count);
};

#endif
