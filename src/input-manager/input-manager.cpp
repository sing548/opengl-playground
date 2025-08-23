#include "input-manager.h"

void InputManager::UpdateKey(int key, int action) {
    keys_[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
}

bool InputManager::IsKeyPressed(int key) const {
    auto it = keys_.find(key);
    return it != keys_.end() && it->second;
}
