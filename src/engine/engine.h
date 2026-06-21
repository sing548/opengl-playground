#ifndef ENGINE_H
#define ENGINE_H

#include <glm/glm.hpp>

#include "window/window.h"
#include "camera/camera.h"
#include "models/model.h"
#include "input-manager/input-manager.h"
#include "rendering/renderer.h"
#include "models/asset-manager.h"

#include "systems/i-gameplay-system.h"

#include "../game/game-world/game-world.h"

#include <map>
#include <string>
#include <thread>

enum class EngineMode { Standalone, Server, Client };

class NetworkBridge;
class TerrainHandler;

class Engine {
public:

    Engine(EngineMode config, const std::string& serverUrl = "", int port = -1);
    ~Engine();
    void BasicLevel();
    void Run();
    AssetManager& GetAssMan();
    Window& GetWindow() { return *window_; };

    void AddGameplaySystem(std::unique_ptr<IGameplaySystem> system) { systems_.push_back(std::move(system)); }
    void AddSceneRenderable(std::unique_ptr<ISceneRenderable> r);
    void AddMaterial(uint16_t id, std::unique_ptr<Material> m) { materials_.emplace(id, std::move(m)); }
    Material* GetMaterial(uint16_t id) { return materials_.at(id).get(); }
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
    
    std::vector<std::unique_ptr<IGameplaySystem>> systems_;
    std::vector<std::unique_ptr<ISceneRenderable>> sceneRenderables_;
    std::unordered_map<uint16_t, std::unique_ptr<Material>> materials_;

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

    void ExecuteSystems(GameplayPhase phase, float dT);
    void HandleLogic(float deltaTime);
    void ReconcileNetwork();
    void UpdateServerNetworking();
    void UpdateClientNetworking();
    void CollectInputs(float deltaTime);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void AddNewPlayer(uint32_t id);
    void RotateModel(unsigned int id, const glm::vec3& change);

    void SortSystems();
    
    //------------------- TEMP, will be moved to more appropriate class ---------------------
    std::tuple<RenderList, FrameGlobals> BuildRenderList();

    //------------- TEMP, only for testing terrain while implementing
    std::unique_ptr<TerrainHandler> terrainHandler_;
};

#endif
