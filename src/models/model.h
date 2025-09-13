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

class Model
{
public:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    std::string directory;
    bool gammaCorrection;

    float radius;

    Model(float radius, PhysicalInfo pi);
    Model(std::string const &path, PhysicalInfo pi, bool gamma = false, float radius = 0.7);

    void Draw(Shader shader);
    void DrawHitbox(Shader shader);
    glm::vec3& GetPosition();
    glm::vec3& GetScale();
    glm::vec3& GetRotation();
    glm::vec3& GetOrientation();
    glm::vec3& GetSpeed();
    glm::vec3& GetRotationSpeed();

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

    void LoadModel(std::string const &path);
    void ProcessNode(aiNode *node, const aiScene *scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
    unsigned int CreateBlackTexture();
};

#endif
