#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "../shaders/shader.h"
#include "asset-manager.h"
#include "../helpers/file-helper.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <array>
#include <string_view>

struct PhysicalInfo {
    glm::vec3 position_;
    glm::vec3 scale_;
    glm::vec3 rotation_;
    glm::vec3 orientation_;
    glm::vec3 baseOrientation_;
    glm::vec3 speed_;
    glm::vec3 rotationSpeed_;
};

enum ModelType
{
    PLAYER,
    SHOT,
    Count
};

class Model
{
public:
    ModelType type_;
    std::string path_;
    float radius_;
    
    std::vector<std::shared_ptr<Mesh>> meshes;
    bool gammaCorrection;

    
    Model(float radius, PhysicalInfo pi, ModelType type);
    Model(std::string const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, bool gamma = false, float radius = 0.7);
    Model(std::string_view const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, bool gamma = false, float radius = 0.7);
    
    void Draw(Shader shader);
    void DrawHitbox(Shader shader);
    glm::vec3 GetPosition() const;
    glm::vec3 GetScale() const;
    glm::vec3 GetRotation() const;
    glm::vec3 GetOrientation() const;
    glm::vec3 GetBaseOrientation() const;
    glm::vec3 GetSpeed() const;
    glm::vec3 GetRotationSpeed() const;
    std::string GetPath() const;
    float GetRadius() const;
    
    void SetPosition(glm::vec3 position);
    void SetScale(glm::vec3 scale);
    void SetRotation(glm::vec3 rotation);
    void SetOrientation(glm::vec3);
    void SetBaseOrientation(glm::vec3 orientation);
    void SetSpeed(glm::vec3 speed);
    void SetRotationSpeed(glm::vec3 speed);
    
    void CreateHitboxSphere();
    
    static Mesh* hitboxSphere_;
    static bool hitboxSphereInitialized_;

    static std::string GetModelPath(ModelType type);
    
private:

    static const std::array<std::filesystem::path, static_cast<size_t>(ModelType::Count)> ModelPaths;

    PhysicalInfo physicalInfo_;
};

#endif
