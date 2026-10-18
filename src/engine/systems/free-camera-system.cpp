#include "free-camera-system.h"

#include "system-structs.h"
#include "../window/window.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

void FreeCameraSystem::Update(SystemsContext& ctx)
{
    if (!ctx.settings.freeCamera) 
    {
        if (mouseLook_)
        {
            glfwSetInputMode(ctx.window.Get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        return;
    }

    if (mouseLook_ && rawMan_.KeyDown(GLFW_KEY_F3) && !f3downlast_)
    {
        glfwSetInputMode(ctx.window.Get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseLook_ = !mouseLook_;
    }
    else if (!mouseLook_ && rawMan_.KeyDown(GLFW_KEY_F3) && !f3downlast_)
    {
        glfwSetInputMode(ctx.window.Get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse_ = true;
        mouseLook_ = !mouseLook_;
    }

    f3downlast_ = rawMan_.KeyDown(GLFW_KEY_F3);

    auto& cam = ctx.window.GetCamera();
    const glm::vec3& cameraUp = cam.GetCameraUp();
    const glm::vec3& worldUp = cam.GetWorldUp();
    const glm::vec3& cameraPos = cam.GetPosition();
    glm::vec3& cameraRight = cam.GetCameraRight();

    glm::vec3 front = glm::normalize(glm::cross(cameraUp, cameraRight));

    bool forward = rawMan_.KeyDown(GLFW_KEY_W);
    bool backward = rawMan_.KeyDown(GLFW_KEY_S);
    bool left = rawMan_.KeyDown(GLFW_KEY_A);
    bool right = rawMan_.KeyDown(GLFW_KEY_D);
    bool up = rawMan_.KeyDown(GLFW_KEY_SPACE);
    bool down = rawMan_.KeyDown(GLFW_KEY_LEFT_CONTROL);
    bool sprint = rawMan_.KeyDown(GLFW_KEY_LEFT_SHIFT);

    if (mouseLook_ || rawMan_.GetMouseDown(GLFW_MOUSE_BUTTON_RIGHT))
        ProcessMouseInput(cam, front, cameraRight);
    
    float scroll = static_cast<float>(rawMan_.GetScroll());
    if (scroll != 0.0f)
        moveSpeed_ *= 1.0f + scroll;

    glm::vec3 pos = cameraPos;
    float speed = moveSpeed_ * (sprint ? 5.0f : 1.0f) * ctx.dT;

    if (forward)
        pos += front * speed;
    if (backward)
        pos -= front * speed;
    if (right)
        pos += cameraRight * speed;
    if (left)
        pos -= cameraRight * speed;
    if (up)
        pos += worldUp * speed;
    if (down)
        pos -= worldUp * speed;

    cam.SetBasis(pos , front, worldUp);
}

void FreeCameraSystem::ProcessMouseInput(Camera& cam, glm::vec3& front, glm::vec3& right)
{
    auto [xpos, ypos] = rawMan_.GetMousePos();
    float fxpos = static_cast<float>(xpos);
    float fypos = static_cast<float>(ypos);

    if (firstMouse_)
    {
        lastXPos_ = fxpos;
        lastYPos_ = fypos;
        firstMouse_ = false;
    }

    lastXOffset_ = fxpos - lastXPos_;
    lastYOffset_ = lastYPos_ - fypos;

    lastXPos_ = fxpos;
    lastYPos_ = fypos;

    front = glm::angleAxis(-lastXOffset_ * mouseXSensitivity_, cam.GetWorldUp()) * front;
    auto can = glm::angleAxis(lastYOffset_ * mouseYSensitivity_, right) * front;

    if (glm::abs(glm::dot(can, cam.GetWorldUp())) < 0.99f)
        front = can;

    right = glm::normalize(glm::cross(front, cam.GetWorldUp()));
    front = glm::normalize(front);
}
