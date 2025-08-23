#pragma once

#include <unordered_map>
#include <GLFW/glfw3.h>

class InputManager {
public:
    void UpdateKey(int key, int action);
    bool IsKeyPressed(int key) const;
private:
    std::unordered_map<int, bool> keys_;
};
