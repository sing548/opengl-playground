#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

class Model
{
public:
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    std::string directory;
    bool gammaCorrection;

    Model(std::string const &path, bool gamma = false) ;

    void Draw(Shader shader);
    glm::vec3 GetPosition();
    void SetPosition(glm::vec3 position);

private:
    glm::vec3 position_;

    void LoadModel(std::string const &path);
    void ProcessNode(aiNode *node, const aiScene *scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
    unsigned int CreateBlackTexture();
};

#endif
