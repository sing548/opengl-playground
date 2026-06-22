#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "stb/stb_image.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "mesh.h"

class AssetManager {
public:
    unsigned int LoadTexture(const std::string& fullPath, bool srgb = false);
    std::vector<std::shared_ptr<Mesh>> LoadModel(const std::string& path);
private:
    std::unordered_map<std::string, unsigned int> textureCache_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Mesh>>> modelCache_;

    bool HasModel(const std::string& path) { return modelCache_.find(path) != modelCache_.end(); };
    bool HasTexture(const std::string& fullPath) { return textureCache_.find(fullPath) != textureCache_.end(); };
    void LoadModelInternal(const std::string& path);
    void ProcessNode(aiNode *node, const aiScene *scene, const std::string& directory, const std::string&  path);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::string& directory);
    unsigned int TextureFromFile(const std::string& fullPath, bool gamma = false);
    unsigned int CreateBlackTexture();
};

#endif
