#include "model.h"

Mesh* Model::hitboxSphere_ = nullptr;
bool Model::hitboxSphereInitialized_ = false;

Model::Model(float radius, PhysicalInfo pi, ModelType type) : gammaCorrection(false), 
	physicalInfo_ (pi), radius(radius), type_(type)
{
	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

Model::Model(std::string const &path, PhysicalInfo pi, AssetManager& assMan, ModelType type, bool gamma, float radius) : gammaCorrection(gamma), physicalInfo_(pi), radius(radius), type_(type)
{
	meshes = assMan.LoadModel(path);
	
	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

void Model::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i]->Draw(shader);
}

void Model::DrawHitbox(Shader shader)
{
	hitboxSphere_->Draw(shader);
}

glm::vec3& Model::GetPosition()
{
	return physicalInfo_.position_;
}

glm::vec3& Model::GetScale()
{
	return physicalInfo_.scale_;
}

glm::vec3& Model::GetRotation()
{
	return physicalInfo_.rotation_;
}

glm::vec3& Model::GetOrientation()
{
	return physicalInfo_.orientation_;
}

glm::vec3& Model::GetSpeed()
{
	return physicalInfo_.speed_;
}

glm::vec3& Model::GetRotationSpeed()
{
	return physicalInfo_.rotationSpeed_;
}

void Model::SetPosition(glm::vec3 position)
{
	this->physicalInfo_.position_ = position;
}

void Model::SetScale(glm::vec3 scale) 
{
	this->physicalInfo_.scale_ = scale;
}

void Model::SetRotation(glm::vec3 rotation)
{
	physicalInfo_.rotation_ = rotation;

    // Convert Euler angles to quaternion
    glm::quat q = glm::quat(physicalInfo_.rotation_);

    // Rotate the *original mesh-facing direction*
    physicalInfo_.orientation_ = q * physicalInfo_.baseOrientation_;
}

void Model::SetBaseOrientation(glm::vec3 orientation)
{
	this->physicalInfo_.orientation_ = orientation;
	this->physicalInfo_.baseOrientation_ = orientation;
}

void Model::SetSpeed(glm::vec3 speed)
{
	this->physicalInfo_.speed_ = speed;
}

void Model::SetRotationSpeed(glm::vec3 speed)
{
	this->physicalInfo_.rotationSpeed_ = speed;
}

void Model::CreateHitboxSphere()
{
	unsigned int sectorCount = 32;
	unsigned int stackCount = 16;

	std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float radius = 1.0f;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount; 
        float xy = cos(stackAngle);
        float z = sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;

            Vertex v{};
            v.Position = glm::vec3(xy * cos(sectorAngle), xy * sin(sectorAngle), z) * radius;
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
