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
uniform float snowStart;
uniform float snowEnd;
uniform float rockStart;
uniform float rockEnd;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform sampler2D grassTex, rockTex, snowTex;
uniform sampler2D grassNormal, rockNormal, snowNormal;
uniform float texScale;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);
vec3 TriplanarTexture(sampler2D tex, vec3 worldPos, vec3 n, vec3 blend);
vec3 TriplanarNormals(sampler2D normalMap, vec3 worldPos, vec3 wN, vec3 blend);

void main()
{
	// use if needed
    //float tile = 1.0;
    //float c = mod(floor(FragPos.x / tile) + floor(FragPos.z / tile), 2.0);
	//vec3 albedo = mix(vec3(0.15), vec3(0.6), c);

	vec3 N = normalize(Normal);

	vec3 blend = abs(N);
	blend = pow(blend, vec3(8.0));
	blend /= (blend.x + blend.y + blend.z);

	vec3 grass = TriplanarTexture(grassTex, FragPos, N, blend);//vec3(0.1, 0.3, 0.05);
	vec3 rock = TriplanarTexture(rockTex, FragPos, N, blend);//vec3(0.3, 0.3, 0.3);
	vec3 snow = TriplanarTexture(snowTex, FragPos, N, blend);//vec3(0.8, 0.8, 0.8);

	float slope = 1.0 - N.y;

	float snowyness = smoothstep(snowStart, snowEnd, FragPos.y);
	vec3 ground = mix(grass, snow, snowyness);

	float rockyness = smoothstep(rockStart, rockEnd, slope);
	vec3 albedo = mix(ground, rock, rockyness);

    vec3 viewDir = normalize(viewPos - FragPos);

	vec3 grassN = TriplanarNormals(grassNormal, FragPos, N, blend);
	vec3 rockN = TriplanarNormals(rockNormal, FragPos, N, blend);
	vec3 snowN = TriplanarNormals(snowNormal, FragPos, N, blend);
	
	vec3 groundN = mix(grassN, snowN, snowyness);
	vec3 mappedN = normalize(mix(groundN, rockN, rockyness));

    vec3 result = CalcDirLight(dirLight, mappedN, viewDir, albedo);

    for (int i = 0; i < numPointLights; i++)
		result += CalcPointLight(pointLights[i], mappedN, FragPos, viewDir, albedo);

    vec3 color = result;

	float dist = length(viewPos - FragPos);
	float fogStrength = smoothstep(fogStart, fogEnd, dist);
	color = mix(color, fogColor, fogStrength);

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

vec3 TriplanarTexture(sampler2D tex, vec3 worldPos, vec3 n, vec3 blend)
{
	vec3 x = texture(tex, worldPos.zy * texScale).rgb;
	vec3 y = texture(tex, worldPos.xz * texScale).rgb;
	vec3 z = texture(tex, worldPos.xy * texScale).rgb;

	return x * blend.x + y * blend.y + z * blend.z;
}

vec3 TriplanarNormals(sampler2D normalMap, vec3 worldPos, vec3 wN, vec3 blend)
{
	// https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
	vec3 nX = texture(normalMap, worldPos.zy * texScale).xyz * 2.0 - 1.0;
	vec3 nY = texture(normalMap, worldPos.xz * texScale).xyz * 2.0 - 1.0;
	vec3 nZ = texture(normalMap, worldPos.xy * texScale).xyz * 2.0 - 1.0;

	nX = vec3(nX.xy + wN.zy, abs(nX.z) * wN.x);
	nY = vec3(nY.xy + wN.xz, abs(nY.z) * wN.y);
	nZ = vec3(nZ.xy + wN.xy, abs(nZ.z) * wN.z);

	return normalize(nX.zyx * blend.x + nY.xzy * blend.y + nZ.xyz * blend.z);
}