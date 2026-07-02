#version 460 core

layout (location = 0) in vec4 aPosAge;
layout (location = 1) in vec3 aTurnSpeedSide;

uniform mat4 view;
uniform mat4 projection;

out vec3 AgeTurnSpeed;
out float vSide;

void main()
{
    AgeTurnSpeed = vec3(aPosAge.w, aTurnSpeedSide.x, aTurnSpeedSide.y);
    vSide = aTurnSpeedSide.z;
    gl_Position = projection * view * vec4(aPosAge.xyz, 1.0);
}
