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
        } else {
            auto& cam = ctx.window.GetCamera();
            glm::vec3 pos = glm::vec3(0.0f, 60.0f, 0.0f);
            glm::vec3 front = glm::vec3(0.0f, -1.0f, 0.0f);
            glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
            cam.SetBasis(pos, front, up);
        }
    }

    if (!adjust) return;

    if (ctx.settings.thirdPerson) {
        if (ctx.localPlayerId == 0 || !ctx.world.GetScene().ModelExists(ctx.localPlayerId)) return;

        const auto& model = ctx.world.GetScene().GetModelByReference(ctx.localPlayerId);
        const auto& interpolatedPi = model.GetInterpolatedInfo();
        glm::vec3 modelPos = interpolatedPi.position_;
        

        auto& cam = ctx.window.GetCamera();

        // Offset from the model in its local orientation
        glm::vec3 offset = interpolatedPi.rotation_ * model.GetBaseOrientation() * glm::vec3(-8.0f, -8.0f, -8.0f);

        glm::vec3 cameraPos = modelPos + offset + model.GetUp() * 1.5f;
        glm::vec3 front = glm::normalize(modelPos - cameraPos);
        glm::vec3 up = model.GetUp();
    
        cam.SetBasis(cameraPos, front, up);
    } else {
        auto& camera = ctx.window.GetCamera();
        auto currentPosition = camera.GetPosition();
        float minHeight = 20.0f;
        float maxHeight = 100.0f;

        float fovyRadians = glm::radians(camera.GetZoom());
        float aspect = (float) ctx.window.GetSize().width / (float) ctx.window.GetSize().height;

        float halfFovTan = tanf(fovyRadians * 0.5f);

        // Required heights in each axis
        float requiredX = (abs(ctx.world.GetScene().currentFurthestPosition.x) + 1.5f) / (halfFovTan * aspect);
        float requiredZ = (abs(ctx.world.GetScene().currentFurthestPosition.z) + 1.5f)/ halfFovTan;

        // Take the larger
        float h = std::max(requiredX, requiredZ);

        // Clamp
        h = std::clamp(h, minHeight, maxHeight);

        float oldHeight = camera.GetPosition().y;

        float changeRate = 0.5f;

        if (abs(h - oldHeight) > changeRate) {
            if (h > oldHeight) h = oldHeight + changeRate;
            if (h < oldHeight) h = oldHeight - changeRate;
        }

        camera.SetPosition(glm::vec3(0, h, 0));
    }
}
