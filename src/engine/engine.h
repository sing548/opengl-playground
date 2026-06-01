#ifndef ENGINE_H
#define ENGINE_H

#include <glm/glm.hpp>

#include "window/window.h"
#include "camera/camera.h"
#include "models/model.h"
#include "input-manager/input-manager.h"
#include "rendering/renderer.h"
#include "models/asset-manager.h"

#include "../game/systems/camera-system.h"
#include "../game/systems/npc-system.h"
#include "../game/systems/physics-system.h"
#include "../game/systems/player-system.h"
#include "../game/systems/shot-system.h"
#include "../game/game-world/game-world.h"

#include "../engine/rendering/materials/model-material.h"
#include "../engine/rendering/materials/hitbox-material.h"

#include <map>
#include <string>
#include <thread>

enum class EngineMode { Standalone, Server, Client };

class NetworkBridge;

class Engine {
public:

    Engine(EngineMode config, const std::string& serverUrl = "", int port = -1);
    ~Engine();
    void BasicLevel();
    void Run();
    AssetManager& GetAssMan();

private:
    const unsigned int WIDTH = 1920;
    const unsigned int HEIGHT = 1080;

    uint32_t playerId_;

    std::unique_ptr<Window> window_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<AssetManager> assMan_;

    std::unique_ptr<NetworkBridge> netwBridg_;

    GameWorld gameWorld_;

    NpcSystem npcSystem_; 
    PhysicsSystem physicsSystem_;
    PlayerSystem playerSystem_;
    ShotSystem shotSystem_;
    CameraSystem cameraSystem_;

    void ChangeSetting(std::string key, bool value);
    glm::vec2 maxScreenSize_;
    glm::vec3 currentFurthestPosition;

    std::unordered_map<std::string, bool> settings_;
    std::unordered_map<uint32_t, InputState> previousInputStates_;
    std::unordered_map<uint32_t, InputState> currentInputStates_;

    // only used in client mode for replay
    std::unordered_map<uint32_t, InputState> currentStateAsMap_;
    std::unordered_map<uint32_t, InputState> previousStateAsMap_;
    std::vector<uint32_t> killedPlayers_;

    bool m_bNetworking = false;
    bool m_bServer = false;

    void HandleLogic(float deltaTime);
    void ReconcileNetwork();
    void UpdateServerNetworking();
    void UpdateClientNetworking();
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
