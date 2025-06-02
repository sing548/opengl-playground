#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "../shaders/shader.h"
#include "../camera/camera.h"
#include "../models/model.h"
#include "../models/scene.h"

class Window
{

public:
    Window(unsigned int width, unsigned int height, const char* title = "Test Window");
    ~Window();

    GLFWwindow* get();
    void draw(Scene scene);
    void processInput(float deltaTime);

private:
    int height_, width_;
    unsigned int framebuffer_;
    unsigned int textureColorbuffer_;
    unsigned int quadVAO_, quadVBO_;
    unsigned int rbo_;
    float lastX_, lastY_;
    bool firstMouse_;

    unsigned int groundVAO_, groundVBO_;

    GLFWwindow* window_;
    glm::vec4 backgroundRGBA_;
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> modelShader_;
    std::unique_ptr<Camera> camera_;
    
    void setCallbacks();
    void mouse_callback(double xpos, double ypos);
    static void error_callback(int error, const char* description);
};

#endif