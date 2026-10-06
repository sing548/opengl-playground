#include "i-terrain-handler.h"

class TerrainHandlerStud : public ITerrainHandler
{
public:
    TerrainHandlerStud() { };
    ~TerrainHandlerStud() override = default;
    void UpdateStreaming(const glm::vec3&) override { };
    TerrainCollision CheckCollision(glm::vec3, float) override 
    { 
        TerrainCollision col
        {
            false,
            0.0f,
            { 0.0f, 0.0f, 0.0f}
        };

        return col;
    };
    std::vector<DrawCommand> BuildDrawCommands(RenderPass) override 
    {
        std::vector<DrawCommand> dc;
        return dc; 
    };
};
