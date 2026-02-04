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

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

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
    NPC,
    OBJECT
};

class Model
{
public:
    ModelType type_;
    
    std::vector<std::shared_ptr<Mesh>> meshes;
    bool gammaCorrection;

    float radius;

    Model(float radius, PhysicalInfo pi, ModelType type);
    Model(std::string const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, bool gamma = false, float radius = 0.7);

    void Draw(Shader shader);
    void DrawHitbox(Shader shader);
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetScale() const;
    const glm::vec3& GetRotation() const;
    const glm::vec3& GetOrientation() const;
    const glm::vec3& GetBaseOrientation() const;
    const glm::vec3& GetSpeed() const;
    const glm::vec3& GetRotationSpeed() const;

    void SetPosition(glm::vec3 position);
    void SetScale(glm::vec3 scale);
    void SetRotation(glm::vec3 rotation);
    void SetBaseOrientation(glm::vec3 orientation);
    void SetSpeed(glm::vec3 speed);
    void SetRotationSpeed(glm::vec3 speed);

    void CreateHitboxSphere();

    static Mesh* hitboxSphere_;
    static bool hitboxSphereInitialized_;

private:
    PhysicalInfo physicalInfo_;
};

#endif
