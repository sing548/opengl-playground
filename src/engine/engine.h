#include "../window/window.h"
#include "../camera/camera.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../input-manager/input-manager.h"
#include "../rendering/renderer.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../models/asset-manager.h"

#include <map>


class Engine {
public:

    Engine();
    void SetupScene(std::vector<Model>);
    void Run();
    AssetManager& GetAssMan();


private:
    const unsigned int WIDTH = 1920;
    const unsigned int HEIGHT = 1080;

    unsigned int lastShot = 0;
    bool shotReleased = true;
    
    std::unique_ptr<Window> window_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<AssetManager> assMan_;

    void ChangeSetting(std::string key, bool value);
    glm::vec2 maxScreenSize_;
    glm::vec3 currentFurthestPosition;

    std::map<std::string, bool> settings_;

    void HandleLogic(float deltaTime);
    void CheckHits();
    void HandleInput(float deltaTime);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void MoveModel(unsigned int id, glm::vec3 change);
    void RotateModel(glm::vec3 change);

    void MoveModels();
    void Shoot(Model shooter);
    void AdjustCamera();
};
