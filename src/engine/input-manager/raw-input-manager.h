#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include "../helpers/file-helper.h"

class RawInputManager 
{
public:
    RawInputManager(GLFWwindow* window) : window_(window) 
    {
        auto path = std::filesystem::path(FileHelper::GetAssetsDir()) / "various" / "gamecontrollerdb.txt";

        std::ifstream file(path);

        if (!file)
        {
            std::cerr << "Failed to open gamecontrollerdb.txt" << path << std::endl;
            return;
        }

        std::stringstream ss;
        ss << file.rdbuf();
        std::string map = ss.str();

        if (glfwUpdateGamepadMappings(map.c_str()) == GLFW_FALSE)
            std::cerr << "glfwUpdateGamepadMappings error" << std::endl;
    };

    bool KeyDown(int key) 
    { 
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }
    
    bool GamepadButtonDown(int key)
    {
        if (!gamepadConnected_) return false;

        return gs_.buttons[key] == GLFW_PRESS;
    }

    std::pair<bool, float> GamepadAxis(int axis)
    {
        if (!gamepadConnected_) return std::pair(false, 0.0f);

        return std::pair(true, gs_.axes[axis]);
    }

    bool GamepadConnected() 
    { 
        gamepadConnected_ = false;

        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gs_))
        {
            gamepadConnected_ = true;
        }

        return gamepadConnected_;
    }

private:
    GLFWwindow* window_;
    GLFWgamepadstate gs_;
    bool gamepadConnected_ = false;
};

/*
int  glfwJoystickPresent(int jid);       // GLFW_JOYSTICK_1 .. _16
int  glfwJoystickIsGamepad(int jid);     // true if GLFW has an SDL mapping for it
int  glfwGetGamepadState(int jid, GLFWgamepadstate* state);   // GLFW_TRUE on success
*/
