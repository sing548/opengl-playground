#version 330 core
out vec4 FragColor;
/*
struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
}

stuct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
}

#define NR_POINT_LIGHTS 4
*/

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;

void main()
{
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	vec3 result = (ambient + diffuse) * texture(texture_diffuse1, TexCoords).rgb;
	FragColor = vec4(result, 1.0);
}
