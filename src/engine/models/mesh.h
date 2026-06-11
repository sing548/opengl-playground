#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../shaders/shader.h"

#include <vector>
#include <string>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
public:
    std::vector<Vertex>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<Texture>		textures;
	unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	// https://en.cppreference.com/cpp/language/rule_of_three
	~Mesh();
	Mesh(const Mesh&)			 = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;

    void Draw(Shader &shader) const;
private:
    unsigned int VBO_, EBO_;

    void SetupMesh();
};

#endif
