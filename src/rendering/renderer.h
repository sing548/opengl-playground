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

    void Draw(const Scene& scene, const Camera& camera, unsigned int width, unsigned int height);
    void ToggleHitboxes();
private:
    unsigned int framebuffer_, textureColorbuffer_;
    unsigned int rbo_, quadVAO_, quadVBO_;
    bool showHitboxes;
    glm::vec4 backgroundRGBA_;
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> modelShader_;
    std::unique_ptr<Shader> hitboxShader_;
};

#endif
