#ifndef SKY_H
#define SKY_H

#include <vector>
#include <string>
#include <memory>

#include "stb/stb_image.h"
#include <glad/glad.h>

#include "../render-list.h"
#include "../i-scene-rendererable.h"

#include "../../shaders/shader.h"

class Sky : public ISceneRenderable
{
public:
    Sky(std::vector<std::string> facePaths);
    ~Sky();

    void Render(const FrameGlobals& globals) override;
    RenderPass GetRenderPass() { return RenderPass::Skybox; };
    int GetOrder() override { return 10; };
private:
    std::unique_ptr<Shader> shader_;
    unsigned int vao_, vbo_;
    unsigned int cubemap_;

    unsigned int LoadCubemap(std::vector<std::string> faces);
};

#endif