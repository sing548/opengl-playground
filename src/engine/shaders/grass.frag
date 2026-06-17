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
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

#define MAX_POINT_LIGHTS 128

in vec3 FragPos;

in vec3 RotatedNormal1;
in vec3 RotatedNormal2;
in float Side;
in float HeightPercent;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;

const float shininess = 32.0;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float heightPercent);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float heightPercent);

void main()
{
    vec3 baseColor = vec3(0.1, 0.4, 0.05);
    vec3 tipColor = vec3(0.6, 0.6, 0.1);
    vec3 color = mix(baseColor, tipColor, HeightPercent);

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 N = mix(RotatedNormal1, RotatedNormal2, Side);
    N = normalize(N);

    vec3 result = CalcDirLight(dirLight, N, viewDir, color, HeightPercent);

    for (int i = 0; i < numPointLights; i++)
        result += CalcPointLight(pointLights[i], N, FragPos, viewDir, color, HeightPercent);

	float dist = length(viewPos - FragPos);
	float fogStrength = smoothstep(fogStart, fogEnd, dist);
	result = mix(result, fogColor, fogStrength);

    FragColor   = vec4(result, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float heightPercent)
{
	vec3 lightDir = normalize(-light.direction);
	float diff = abs(dot(normal, lightDir));

	vec3 N = (dot(normal, viewDir) < 0.0) ? -normal : normal;
	vec3 H = normalize(lightDir + viewDir);
	float spec = pow(max(dot(N, H), 0.0), shininess);

	vec3 ambient = light.ambient * albedo * mix(0.3, 1.0, heightPercent);
	vec3 diffuse = light.diffuse * diff * albedo;
	vec3 specular = light.specular * spec * heightPercent;
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float heightPercent)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = abs(dot(normal, lightDir));

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * albedo;
	vec3 diffuse = light.diffuse * diff * albedo;

	vec3 N = (dot(normal, viewDir) < 0.0) ? -normal : normal;
	vec3 H = normalize(lightDir + viewDir);
	float spec = pow(max(dot(N, H), 0.0), shininess);
	vec3 specular = light.specular * spec;
	
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	specular *= heightPercent;

	return (ambient + diffuse + specular);
}
