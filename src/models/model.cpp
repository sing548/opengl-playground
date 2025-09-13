#include "model.h"

Mesh* Model::hitboxSphere_ = nullptr;
bool Model::hitboxSphereInitialized_ = false;

Model::Model(float radius, PhysicalInfo pi) : gammaCorrection(false), 
	physicalInfo_ (pi), radius(radius)
{
	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

Model::Model(std::string const &path, PhysicalInfo pi, bool gamma, float radius) : gammaCorrection(gamma), physicalInfo_(pi), radius(radius)
{
	LoadModel(path);

	if (!hitboxSphereInitialized_)
		CreateHitboxSphere();
}

void Model::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
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

void Model::LoadModel(std::string const &path)
{
	Assimp::Importer import;

	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode *node, const aiScene *scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		
		vertex.Position = vector;
	
        if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
	
        if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
	
            // Texture Coords
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
	
            // tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
	
            // bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
	
        vertices.push_back(vertex);
	}
	
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	
    if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// diffuse maps
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// specular maps
		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// normal maps
		std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// height maps
		std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}
	
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
	
        for (unsigned int j = 0; j < textures.size(); j++)
		{
			if (std::strcmp(textures[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures[j]);
				skip = true;
				break;
			}
		}
	
        if (!skip)
		{
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures.push_back(texture);
		}
	}

	if (textures.size() == 0)
	{
		Texture dummy;
        dummy.id = CreateBlackTexture();  // <-- You define this function below
        dummy.type = typeName;
        dummy.path = "dummy_black";
        textures.push_back(dummy);
        return textures;
	}
	
    return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	
    if (data)
	{
		GLenum format;
		
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
	
        glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

    return textureID;
}

unsigned int Model::CreateBlackTexture()
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char black[] = { 0, 0, 0 }; // RGB black pixel

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, black);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}
