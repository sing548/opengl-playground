#ifndef CON_TRAIL_H
#define CON_TRAIL_H

#include <deque>
#include <chrono>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../../../engine/rendering/i-scene-rendererable.h"

#include "../../game-world/game-world.h"

struct ChunkRegion;
struct FrameGlobals;

class Shader;

class ConTrail : public ISceneRenderable
{
private:
struct QueueData {
    std::array<glm::vec3, 4> Positions;
    float Angle;
    float Velocity;
    std::chrono::steady_clock::time_point CreationTime;
};

struct ConVertData {
    glm::vec3 position;
    float age;
    float angle;
    float velocity;
    float side;
};

public:
   ConTrail(GameWorld& world);
   ~ConTrail();

   void Render(const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings) override;
    RenderPass GetRenderPass() override { return RenderPass::Transparent; };
    int GetOrder() override { return 30; };
private:
    const int TRAIL_SEGMENT_LENGTH = 1;
    const int TRAIL_SEGMENT_COUNT = 450;
    const float MAX_AGE = 5.0f;

    std::unordered_map<uint32_t, std::deque<QueueData>> trails_;
    std::unique_ptr<Shader> shader_;
    unsigned int vao_, pointsVBO_, pointDataVBO_;
    GameWorld& world_;

    void BuildTrails();
    void ExtendDeque(uint32_t id);

    std::array<glm::vec3, 4> wingtipOffsets_;
};

#endif
