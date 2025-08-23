#include "../window/window.h"
#include "../camera/camera.h"
#include "../models/scene.h"
#include "../models/model.h"
#include "../input-manager/input-manager.h"
#include "../rendering/renderer.h"
#include "../models/scene.h"
#include "../models/model.h"

class Engine {
public:

    Engine();
    void SetupScene(std::vector<Model>);
    void Run();
    private:
    const unsigned int WIDTH = 1920;
    const unsigned int HEIGHT = 1080;
    
    std::unique_ptr<Window> window_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<Camera> camera_;
    Renderer* renderer_;
    std::unique_ptr<Scene> scene_;

    void HandleLogic();
};
