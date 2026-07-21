#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <memory>

#include "../camera/camera.h"

struct WindowSize
{
    unsigned int width;
    unsigned int height;
};

class Window
{

public:
    Window(unsigned int width, unsigned int height, std::unique_ptr<Camera> camera, const char* title = "Test Window");
    ~Window();

    GLFWwindow* Get();
    Camera& GetCamera();
    void SetupFrame();
    void SwapBuffers();
    WindowSize GetSize();
    void HandleInput(float deltaTime);
    void ResizeWindow(unsigned int width, unsigned int height);
    bool WasWindowResized() { return windowResized_; };
    void ClearResizedFlag() { windowResized_ = false; };

private:
    float lastX_, lastY_;
    unsigned int firstMouse_ = 20;
    bool windowResized_ = false;

    GLFWwindow* window_;
    WindowSize size_;
    glm::vec4 backgroundRGBA_;
    std::unique_ptr<Camera> camera_;
    
    static void error_callback(int error, const char* description);
};

#endif