#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;

	float constant;
	float linear;
	float quadratic;
};

#define MAX_POINT_LIGHTS 128

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);

void main()
{
    float tile = 1.0;
    float c = mod(floor(FragPos.x / tile) + floor(FragPos.z / tile), 2.0);
    vec3 albedo = mix(vec3(0.15), vec3(0.6), c);

    vec3 N = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, N, viewDir, albedo);

    for (int i = 0; i < numPointLights; i++)
		result += CalcPointLight(pointLights[i], N, FragPos, viewDir, albedo);

    vec3 color = result;

    FragColor = vec4(color, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo)
{
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 ambient = light.ambient * albedo;
	vec3 diffuse = light.diffuse * diff * albedo;
	return (ambient + diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * albedo;
	vec3 diffuse = light.diffuse * diff * albedo;
	
	ambient *= attenuation;
	diffuse *= attenuation;

	return (ambient + diffuse);
}
