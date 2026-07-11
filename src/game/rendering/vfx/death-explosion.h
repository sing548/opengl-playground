#ifndef DEATH_EXPLOSION_H
#define DEATH_EXPLOSION_H

#include <chrono>

#include "../../../engine/rendering/i-scene-rendererable.h"

#include "../../game-world/game-world.h"

class DeathExplosion : public ISceneRenderable
{
public:
struct ActiveDeath {
    glm::vec3 position;
    std::chrono::steady_clock::time_point start;
};

    DeathExplosion(GameWorld& world);
    ~DeathExplosion();

    void Render(const FrameGlobals& globals) override;
    RenderPass GetRenderPass() override { return RenderPass::Emissive; };
    int GetOrder() override { return 25; };
private:
    std::unique_ptr<Shader> shader_;
    unsigned int vertIDVBO_, instanceVBO_, indexBufferEBO_, vao_;
    int indexCount_, instanceCount_;

    GameWorld& world_;

    std::vector<ActiveDeath> active_;
    const float DEATH_LINGER = 1.0f;
};

#endif
