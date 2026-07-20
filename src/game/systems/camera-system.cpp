#include "../systems/camera-system.h"

#include "../game-world/game-world.h"
#include "../../engine/window/window.h"

void CameraSystem::Update(SystemsContext& ctx)
{
    bool adjust = ctx.settings.adjustCamera;

    if (thirdPersonActive_ != ctx.settings.thirdPerson)
    {
        thirdPersonActive_ = ctx.settings.thirdPerson;

        if (ctx.settings.thirdPerson) {
            ctx.settings.adjustCamera = true;
            auto& cam = ctx.window.GetCamera();
            cam.Up = cam.WorldUp;
            cam.Pitch = 0.0f;
            cam.UpdateCameraVectors();
        } else {
            auto& cam = ctx.window.GetCamera();
            cam.Up = glm::vec3(0.0f, 0.0f, 1.0f);
            cam.Front = glm::vec3(0.0f, 0.0f, -1.0f);
            cam.Pitch = -90.0f;
            cam.UpdateCameraVectors();
            cam.Position = glm::vec3(0.0f, 60.0f, 0.0f);
        }
    }

    if (!adjust) return;

    if (ctx.settings.thirdPerson) {
        if (ctx.localPlayerId == 0 || !ctx.world.GetScene().ModelExists(ctx.localPlayerId)) return;

        const auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        const auto& interpolatedPi = model.GetInterpolatedInfo();
        glm::vec3 modelPos = interpolatedPi.position_;
        
        // Offset from the model in its local orientation
        glm::vec3 offset = interpolatedPi.rotation_ * model.GetBaseOrientation() * glm::vec3(-8.0f, -8.0f, -8.0f);
        glm::vec3 cameraPos = modelPos + offset + glm::vec3(0.0f, 1.5f, 0.0f);

        auto& cam = ctx.window.GetCamera();
        cam.Up = model.GetUp();
        cam.Pitch = 0.0f;
        cam.UpdateCameraVectors();
    
        // Update camera position
        ctx.window.UpdateCameraPosition(cameraPos);
    
        // Compute direction from camera to model
        glm::vec3 front = glm::normalize(modelPos - cameraPos);
        ctx.window.UpdateCameraOrientation(front);
    } else {
        auto currentPosition = ctx.window.GetCamera().Position;
        float minHeight = 20.0f;
        float maxHeight = 100.0f;

        float fovyRadians = glm::radians(ctx.window.GetCamera().Zoom);
        float aspect = (float) ctx.window.GetSize().width / (float) ctx.window.GetSize().height;

        float halfFovTan = tanf(fovyRadians * 0.5f);

        // Required heights in each axis
        float requiredX = (abs(ctx.world.GetScene().currentFurthestPosition.x) + 1.5f) / (halfFovTan * aspect);
        float requiredZ = (abs(ctx.world.GetScene().currentFurthestPosition.z) + 1.5f)/ halfFovTan;

        // Take the larger
        float h = std::max(requiredX, requiredZ);

        // Clamp
        h = std::clamp(h, minHeight, maxHeight);

        float oldHeight = ctx.window.GetCamera().Position.y;

        float changeRate = 0.5f;

        if (abs(h - oldHeight) > changeRate) {
            if (h > oldHeight) h = oldHeight + changeRate;
            if (h < oldHeight) h = oldHeight - changeRate;
        }

        ctx.window.UpdateCameraPosition(glm::vec3(0, h, 0));
    }
}
