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

struct WindowSize
{
    unsigned int width;
    unsigned int height;
};

class Window
{

public:
    Window(unsigned int width, unsigned int height, const char* title = "Test Window");
    ~Window();

    GLFWwindow* Get();
    Camera GetCamera();
    void SetupFrame();
    void SwapBuffers();
    WindowSize GetSize();
    void HandleInput(float deltaTime);
    void ResizeWindow(unsigned int width, unsigned int height);

private:
    float lastX_, lastY_;
    unsigned int firstMouse_ = 20;

    GLFWwindow* window_;
    WindowSize size_;
    glm::vec4 backgroundRGBA_;
    std::unique_ptr<Camera> camera_;
    
    void setCallbacks();
    void mouse_callback(double xpos, double ypos);
    static void error_callback(int error, const char* description);
};

#endif