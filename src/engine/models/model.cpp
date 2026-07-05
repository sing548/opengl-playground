#include "model.h"

#include <glad/glad.h>
#include "stb_image.h"

#include "../helpers/file-helper.h"

Mesh* Model::hitboxSphere_ = nullptr;
bool Model::hitboxSphereInitialized_ = false;

const std::array<std::filesystem::path, static_cast<size_t>(ModelType::Count)>  Model::ModelPaths = 
{
    std::filesystem::path("models") / "tie2" / "bland-tie.obj",
    std::filesystem::path("models") / "shot" / "longshot.obj"
};


Model::Model(float radius, PhysicalInfo pi, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp) : gammaCorrection(false), 
	physicalInfo_ (pi), radius_(radius), type_(type), baseOrientation_(baseOrientation), baseUp_(baseUp), baseRight_(glm::cross(baseOrientation, baseUp))
{
	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

Model::Model(std::string const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp, bool gamma, float radius) 
    : gammaCorrection(gamma), physicalInfo_(pi), radius_(radius), type_(type), baseOrientation_(baseOrientation), baseUp_(baseUp), baseRight_(glm::cross(baseOrientation, baseUp))
{
    path_ = path;
	meshes_ = assMan.LoadModel(path);
	
	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

Model::Model(std::string_view const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, glm::vec3 baseOrientation, glm::vec3 baseUp, bool gamma, float radius) 
    : gammaCorrection(gamma), physicalInfo_(pi), radius_(radius), type_(type), baseOrientation_(baseOrientation), baseUp_(baseUp), baseRight_(glm::cross(baseOrientation, baseUp))
{
    path_ = path;
    meshes_ = assMan.LoadModel(path_);

    if (!hitboxSphereInitialized_)
        CreateHitboxSphere();
}

void Model::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes_.size(); i++)
		meshes_[i]->Draw(shader);
}

void Model::DrawHitbox(Shader shader)
{
	hitboxSphere_->Draw(shader);
}

const std::vector<std::shared_ptr<Mesh>>& Model::GetMeshes() const
{
    return meshes_;
}

const Mesh* Model::GetHitboxMesh()
{
    return hitboxSphere_;
}

void Model::CreateHitboxSphere()
{
	unsigned int sectorCount = 32;
	unsigned int stackCount = 16;

	std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount; 
        float xy = cos(stackAngle);
        float z = sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;

            Vertex v{};
            v.Position = glm::vec3(xy * cos(sectorAngle), xy * sin(sectorAngle), z) * radius_;
            v.Normal   = glm::normalize(v.Position); // sphere normal
            v.TexCoords = glm::vec2(
                (float)j / sectorCount,
                (float)i / stackCount
            );
            v.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            v.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

            vertices.push_back(v);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0)
                indices.insert(indices.end(), {k1, k2, k1 + 1});
            if (i != (stackCount - 1))
                indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
        }
    }

	hitboxSphere_ = new Mesh(vertices, indices, textures);
	hitboxSphereInitialized_ = true;
}

std::string Model::GetModelPath(ModelType type)
{
    return (FileHelper::GetAssetsDir() / ModelPaths[static_cast<size_t>(type)]).string();
}

