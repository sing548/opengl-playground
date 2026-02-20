#ifndef ENGINE_H
#define ENGINE_H

#include "../window/window.h"
#include "../camera/camera.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../input-manager/input-manager.h"
#include "../rendering/renderer.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../models/asset-manager.h"
#include "../networking/networking.h"

#include "../systems/camera-system.h"
#include "../systems/physics-system.h"
#include "../systems/player-system.h"
#include "../systems/shot-system.h"

#include <map>
#include <thread>

class Engine {
public:

    Engine(int config, const char *serverAddr = nullptr);
    std::vector<Model> BasicLevel();
    void SetupScene(std::vector<Model>);
    void Run();
    AssetManager& GetAssMan();

private:
    const unsigned int WIDTH = 1920;
    const unsigned int HEIGHT = 1080;

    unsigned int lastShot = 0;
    bool shotReleased = true;

    int playerId_;

    std::unique_ptr<Window> window_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<AssetManager> assMan_;

    PhysicsSystem physicsSystem_;
    PlayerSystem playerSystem_;
    ShotSystem shotSystem_;
    CameraSystem cameraSystem_;

    void ChangeSetting(std::string key, bool value);
    glm::vec2 maxScreenSize_;
    glm::vec3 currentFurthestPosition;

    std::map<std::string, bool> settings_;
    std::unordered_map<int, InputState> previousInputStates_;
    std::unordered_map<int, InputState> currentInputStates_;
    std::vector<uint32_t> killedPlayers_;

    bool m_bNetworking = false;
    bool m_bServer = false;
    Networking* networking_ = nullptr;

    void HandleLogic(float deltaTime);
    void ReconcileNetwork();
    void CollectInputs(float deltaTime);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void AddNewPlayer(uint32_t id);
    void RotateModel(unsigned int id, const glm::vec3& change);

    void AdjustCamera();
};

#endif
