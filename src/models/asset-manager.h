#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "mesh.h"
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssetManager {
public:
    std::vector<std::shared_ptr<Mesh>> LoadModel(const std::string& path);

private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<Mesh>>> modelCache_;

    bool HasModel(const std::string& path);
    void LoadModelInternal(const std::string& path);
    void ProcessNode(aiNode *node, const aiScene *scene, const std::string& directory, const std::string&  path);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& directory);
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
    unsigned int CreateBlackTexture();
};

#endif
