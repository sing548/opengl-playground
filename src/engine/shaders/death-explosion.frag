#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 uv;
in float intensity;

void main()
{
    vec3 color = vec3(1.0, 0.2, 0.2);
    float glow = 1.0 - smoothstep(0.0, 1.0, length(uv) * 2.0);
    FragColor = vec4(color * glow, intensity);
    BrightColor = vec4(color * glow, intensity);
}
