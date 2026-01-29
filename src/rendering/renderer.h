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

    void Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height);
    void ToggleHitboxes();
    void ToggleSkyBox();
private:
    unsigned int framebuffer_, textureColorbuffer_;
    unsigned int rbo_, quadVAO_, quadVBO_;
    unsigned int skyboxVAO_, skyboxVBO_;
    unsigned int cubemapTexture_;
    bool showHitboxes_, showSkyBox_;
    glm::vec4 backgroundRGBA_;
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> modelShader_;
    std::unique_ptr<Shader> hitboxShader_;
    std::unique_ptr<Shader> skyboxShader_;

    unsigned int LoadCubemap(std::vector<std::string> faces);
};

#endif
