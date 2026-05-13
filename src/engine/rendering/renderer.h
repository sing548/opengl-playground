#ifndef RENDERER_H
#define RENDERER_H

#include <memory>
#include <filesystem>
#include <map>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../models/mesh.h"
#include "../helpers/file-helper.h"
#include "../rendering/render-list.h"

#include "../rendering/materials/material.h"

#include "../rendering/sky/sky.h"

class Renderer
{
public:
    Renderer(unsigned int width, unsigned int height);

    void Draw(const RenderList& renderList, const FrameGlobals& globals, const std::map<std::string, bool>& settings);

private:
    unsigned int frameFBO_;
    unsigned int colorBuffers_[2];
    unsigned int pingpongFBO_[2];
    unsigned int pingPongColorbuffers_[2];
    unsigned int rbo_, quadVAO_, quadVBO_;
    unsigned int skyboxVAO_, skyboxVBO_;
    unsigned int cubemapTexture_;

    glm::vec4 backgroundRGBA_;
    std::array<GLfloat, 4> sceneClear_;
    std::array<GLfloat, 4> black_; 
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> blurShader_;
    
    std::unique_ptr<Sky> sky_;
    
    void ApplyPassState(RenderPass pass);
    void PostProcessing();
    unsigned int LoadCubemap(std::vector<std::string> faces);
};

#endif
