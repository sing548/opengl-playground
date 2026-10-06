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

in vec2 uv;
in float intensity;
in vec3 FragPos;

uniform mat4 view;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor);

void main()
{
    vec3 color = vec3(0.0, 0.0, 1.0);

    vec2 point = uv * 2.0;
    float r2 = dot(point, point);

    if (r2 > 1.0)
        discard;

    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 cameraBack  = vec3(view[0][2], view[1][2], view[2][2]);

    vec3 n = vec3(point, sqrt(1.0 - r2));
    vec3 normal = normalize(cameraRight * n.x + cameraUp * n.y + cameraBack * n.z);

    vec3 result = CalcDirLight(dirLight, normal, cameraBack, color);

    for (int i = 0; i < numPointLights; i++)
        result += CalcPointLight(pointLights[i], normal, FragPos, cameraBack, color);

    FragColor = vec4(result, 1.0);
    BrightColor = vec4(0.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);

	vec3 ambient = light.ambient * baseColor;
	vec3 diffuse = light.diffuse * diff * baseColor;
	return (ambient + diffuse);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);

	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient * baseColor;
	vec3 diffuse = light.diffuse * diff * baseColor;
	
	ambient *= attenuation;
	diffuse *= attenuation;

	return (ambient + diffuse);
}
