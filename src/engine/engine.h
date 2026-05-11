#ifndef ENGINE_H
#define ENGINE_H

#include <glm/glm.hpp>

#include "../window/window.h"
#include "../camera/camera.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../input-manager/input-manager.h"
#include "../rendering/renderer.h"
#include "../models/asset-manager.h"
#include "../networking/networking.h"

#include "../systems/camera-system.h"
#include "../systems/npc-system.h"
#include "../systems/physics-system.h"
#include "../systems/player-system.h"
#include "../systems/shot-system.h"


#include "../rendering/materials/model-material.h"
#include "../rendering/materials/hitbox-material.h"

#include <map>
#include <thread>

enum class EngineMode { Standalone, Server, Client };

class Engine {
public:

    Engine(EngineMode config, const char *serverAddr = nullptr);
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
    std::unique_ptr<Networking> networking_;

    NpcSystem npcSystem_; 
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

    void HandleLogic(float deltaTime);
    void ReconcileNetwork();
    void CollectInputs(float deltaTime);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void AddNewPlayer(uint32_t id);
    void RotateModel(unsigned int id, const glm::vec3& change);

    void AdjustCamera();


    //------------------- TEMP, will be moved to more appropriate class ---------------------
    std::unique_ptr<Shader> screenShader_;
    std::unique_ptr<Shader> modelShader_;
    std::unique_ptr<Shader> hitboxShader_;
    std::tuple<RenderList, FrameGlobals> BuildRenderList();
    void TempBuildRenderHelpers();
    std::unique_ptr<ModelMaterial> modelMat_;
    std::unique_ptr<HitboxMaterial> hitboxMat_;
};

#endif
