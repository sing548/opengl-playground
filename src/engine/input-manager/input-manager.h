#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>


class InputManager {
public:
    void UpdateKey(int key, int action);
    bool IsKeyPressed(int key) const;

    void BindFloat(int keyA, int keyB, float& val, GLFWwindow* window)
    {
        if (glfwGetKey(window, keyA) == GLFW_PRESS &&
            glfwGetKey(window, keyB) == GLFW_RELEASE)
            val = 1.0f;
        else if (glfwGetKey(window, keyA) == GLFW_RELEASE &&
                 glfwGetKey(window, keyB) == GLFW_PRESS)
            val = -1.0f;
        else
            val = 0.0f;
    }

private:
    std::unordered_map<int, bool> keys_;
};
