#ifndef RENDER_LIST_H
#define RENDER_LIST_H

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

class Mesh;
class Shader;
class Material;

enum class RenderPass : uint16_t {
	Opaque 	= 0,
	Skybox	= 1,
	Debug	= 2
};

struct DrawCommand {
	const Mesh* mesh;
	glm::mat4 	transform;
	Material* 	material;
	glm::vec4 	tint = {1,1,1,1};
	RenderPass 	renderPass = RenderPass::Opaque;
};

struct RenderList {
	std::vector<DrawCommand> commands;
};

struct PointLight {
	glm::vec3 position;
	glm::vec3 ambient	= {0.0f, 0.0f, 0.0f};
	glm::vec3 diffuse	= {0.2f, 0.0f, 0.0f};
	glm::vec3 specular	= {1.0f, 0.0f, 0.0f};
	float constant		= 1.0f;
	float linear		= 0.09f;
	float quadratic		= 0.032f;
};

struct DirectionalLight {
	glm::vec3 direction = {-1.0f, -1.0f, 0.0f};
	glm::vec3 ambient	= {0.15f, 0.16f, 0.175f};
	glm::vec3 diffuse	= {0.4f, 0.42f, 0.45f};
};

struct FrameGlobals {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 cameraPos;
	float	  time;

	DirectionalLight	dirLight;
	std::vector<PointLight> pointLights;
};

#endif
