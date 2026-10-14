#version 460 core

layout (location = 0) in float vertID;
layout (location = 1) in vec4 nInstance;

uniform mat4 view;
uniform mat4 projection;
uniform float particleSize;

out vec2 uv;
out float intensity;
out vec3 FragPos;

void main()
{
    float side = mod(vertID, 2.0);
    float segment = floor(vertID / 2.0);

    float x = (side - 0.5);
    float y = (segment - 0.5);
    
    uv = vec2(x, y);
    intensity = nInstance.w;

    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);

    FragPos = nInstance.xyz + (cameraRight * x + cameraUp * y) * particleSize;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
