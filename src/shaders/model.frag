#version 330 core
out vec4 FragColor;

struct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float costant;
	float linear;
	float quadratic;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform DirLight dirLight;

void main()
{
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(dirLight.direction);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

	vec3 result = (ambient + diffuse) * texture(texture_diffuse1, TexCoords).rgb;
	FragColor = vec4(result, 1.0);
}
