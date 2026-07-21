#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>
#include <string>
#include <vector>
#include <string_view>

#include "mesh.h"
#include "asset-manager.h"
#include "../shaders/shader.h"


struct PhysicalInfo {
    glm::vec3 position_          {0.0f};
    glm::vec3 scale_             {1.0f};
    glm::quat rotation_          {1, 0, 0, 0};
    glm::vec3 velocity_          {0.0f};
    glm::vec3 angularVelocity_   {0.0f};
};

// ToDo: Move in future to make Split engine/game clearer
enum class ModelType : uint32_t
{
    PLAYER,
    SHOT,
    NPC,
    Count
};

class Model
{
public:
    ModelType type_;
    std::string path_;
    float radius_;
    
    bool gammaCorrection;

    
    Model(float radius, PhysicalInfo pi, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp);
    Model(std::string const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp, bool gamma = false, float radius = 0.7);
    Model(std::string_view const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp, bool gamma = false, float radius = 0.7);
    
    void Draw(Shader shader);
    void DrawHitbox(Shader shader);
    glm::vec3 GetPosition() const { return physicalInfo_.position_; };
    glm::vec3 GetScale() const { return physicalInfo_.scale_; };
    glm::quat GetRotation() const { return physicalInfo_.rotation_; };
    glm::vec3 GetUp() const { return GetRotation() * baseUp_; };
    glm::vec3 GetRight() const { return GetRotation() * baseRight_; };
    glm::vec3 GetForward() const { return GetRotation() * baseOrientation_; };
    glm::vec3 GetBaseOrientation() const { return baseOrientation_; };
    glm::vec3 GetBaseUp() const { return baseUp_; };
    glm::vec3 GetBaseRight() const { return baseRight_; };
    glm::vec3 GetVelocity() const { return physicalInfo_.velocity_; };
    glm::vec3 GetRotationSpeed() const { return physicalInfo_.angularVelocity_; };
    std::string GetPath() const { return path_; };
    float GetRadius() const { return radius_; };
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const;
    const static Mesh* GetHitboxMesh();
    
    void SetPosition(glm::vec3 position) { physicalInfo_.position_ = position; };
    void SetScale(glm::vec3 scale) { physicalInfo_.scale_ = scale; };
    void SetRotation(glm::quat rotation) { physicalInfo_.rotation_ = rotation; };
    void RotateBy(glm::quat delta) { physicalInfo_.rotation_ = delta * physicalInfo_.rotation_; };
    void SetVelocity(glm::vec3 speed) { physicalInfo_.velocity_ = speed; };
    void SetRotationSpeed(glm::vec3 speed) { physicalInfo_.angularVelocity_ = speed; };

    const PhysicalInfo& GetPhysicalInfo() const { return physicalInfo_; };
    void SetPreviousInfo(PhysicalInfo pi) { previousInfo_ = pi; };
    const PhysicalInfo& GetPreviousInfo() const { return previousInfo_; };
    void SetInterpolatedInfo(PhysicalInfo pi) { interpolatedInfo_ = pi; };
    void SetInterpolationOffset(glm::vec4 offset) { interpolationOffset_ = offset; };
    const PhysicalInfo& GetInterpolatedInfo() const { return interpolatedInfo_; };
    glm::vec3 GetInterpolatedPosition() const { return interpolatedInfo_.position_; };
    glm::vec4 GetInterpolationOffset() const { return interpolationOffset_; };
    glm::quat GetInterpolatedRotation() const { return interpolatedInfo_.rotation_; };
    
    void CreateHitboxSphere();
    
    static Mesh* hitboxSphere_;
    static bool hitboxSphereInitialized_;

    // ToDo: Move in future to make Split engine/game clearer
    static std::string GetModelPath(ModelType type);
    
private:

    static const std::array<std::filesystem::path, static_cast<size_t>(ModelType::Count)> ModelPaths;

    glm::vec3 baseUp_;
    glm::vec3 baseRight_;
    glm::vec3 baseOrientation_;
    PhysicalInfo physicalInfo_;
    PhysicalInfo previousInfo_;
    PhysicalInfo interpolatedInfo_;
    // Vec3 + remaining blend (time-)factor 1->0
    glm::vec4 interpolationOffset_ = glm::vec4(0.0f);

    std::vector<std::shared_ptr<Mesh>> meshes_;
};

#endif
