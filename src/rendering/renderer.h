#ifndef RENDERER_H
#define RENDERER_H

#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../models/scene.h"
#include "../camera/camera.h"

class Renderer
{
public:
    Renderer(unsigned int width, unsigned int height);
    Renderer(unsigned int width, unsigned int height, bool showHitboxes, bool showSkyBox);

    void Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height, const std::map<std::string, bool>& settings);
    void ToggleHitboxes();
    void ToggleSkyBox();
private:
    unsigned int frameFBO_;
    unsigned int colorBuffers_[2];
    unsigned int pingpongFBO_[2];
    unsigned int pingPongColorbuffers_[2];
    unsigned int rbo_, quadVAO_, quadVBO_;
    unsigned int skyboxVAO_, skyboxVBO_;
    unsigned int cubemapTexture_;
    bool showHitboxes_, showSkyBox_;
    glm::vec4 backgroundRGBA_;
    std::array<GLfloat, 4> sceneClear_;
    std::array<GLfloat, 4> black_; 
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> modelShader_;
    std::unique_ptr<Shader> hitboxShader_;
    std::unique_ptr<Shader> skyboxShader_;
    std::unique_ptr<Shader> blurShader_;

    unsigned int LoadCubemap(std::vector<std::string> faces);
};

#endif
