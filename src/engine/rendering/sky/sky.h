#ifndef SKY_H
#define SKY_H

#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "stb/stb_image.h"
#include <glad/glad.h>

#include "../render-list.h"
#include "../i-scene-rendererable.h"

#include "../../shaders/shader.h"
#include "../../helpers/file-helper.h"

class Sky : public ISceneRenderable
{
public:
    Sky(std::vector<std::string> facePaths);
    ~Sky();

    void Render(const FrameGlobals& globals, const std::unordered_map<std::string, bool>& settings) override;
    RenderPass GetRenderPass() { return RenderPass::Skybox; };
private:
    std::unique_ptr<Shader> shader_;
    unsigned int vao_, vbo_;
    unsigned int cubemap_;

    unsigned int LoadCubemap(std::vector<std::string> faces);
};

#endif