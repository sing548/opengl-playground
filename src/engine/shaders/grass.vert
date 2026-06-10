#version 460 core

layout (location = 0) in float vertID;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec3 randoms;

out vec3 FragPos;

out vec3 RotatedNormal1;
out vec3 RotatedNormal2;
out float Side;
out float HeightPercent;

uniform mat4 view;
uniform mat4 projection;
uniform float time;

const float GRASS_HEIGHT = 1.0;
const float WIDTH  = 0.1;
const float SEGS   = 7.0;

mat3 RotateX(float angle);
mat3 RotateY(float angle);

vec3 mod289(vec3 x);
vec2 mod289(vec2 x);
vec3 permute(vec3 x);
float snoise(vec2 v);

void main()
{
    // Setup
    float side = mod(vertID, 2.0);
    Side = side;
    float segment = floor(vertID / 2.0);
    float currentSegmentLevel = segment / SEGS;
    float angle = randoms.x * 2 * 3.14159;


    // Building geometry
    float randHeight = randoms.y;
    float x = (side - 0.5) * WIDTH * (1.0 - pow(currentSegmentLevel,5));
    float y = currentSegmentLevel * GRASS_HEIGHT * randHeight;

    vec3 local = vec3(x, y, 0.0);

    // Deforming
    float randLean = randoms.z * currentSegmentLevel;
    float randomWindNoise = snoise(vec2(time * 0.3) + offset.xz) * 0.1;
    mat3 leanMat = RotateX(randLean + randomWindNoise);
    vec3 bent = leanMat * local;
    vec3 rotated = RotateY(angle) * bent;

    // snoise is [-1,1] -> angle needs to be 0..2PI
    float windDir = snoise(vec2(time * 0.01) + offset.xz * 0.005);
    windDir = (windDir * 0.5 + 0.5) * 3.14159;
    float windStrength = snoise(vec2(time * 0.25) + offset.xz * 0.025);
    windStrength = (windStrength * 0.5 + 0.5) * 2;

    // Missing: Rotate by height (like randLean) + windStrength in direction of windDir
    float windLean = windStrength * currentSegmentLevel;
    vec3 windBent = RotateY(windDir) * (RotateX(windLean) * (RotateY(-windDir) * rotated));

    // Calculating normal for lighting
    vec3 norm = vec3(0.0, 0.0, 1.0);
    norm = leanMat * norm;
    norm = RotateY(angle) * norm;
    norm = RotateY(windDir) * (RotateX(windLean) * (RotateY(-windDir) * norm));
    RotatedNormal1 = RotateY(3.14159 * 0.3) * norm;
    RotatedNormal2 = RotateY(3.14159 * -0.3) * norm;

    vec3 world = offset + windBent;
    HeightPercent = currentSegmentLevel;
    FragPos = world.xyz;
    gl_Position = projection * view * vec4(world, 1.0);
}

mat3 RotateX(float angle)
{
    float c = cos(angle), s = sin(angle);
    return mat3(
        1.0, 0.0, 0.0,
        0.0, c, s,
        0.0, -s, c
    );
}

mat3 RotateY(float angle)
{
    float c = cos(angle), s = sin(angle);
    return mat3(
        c, 0.0, -s,
        0.0, 1.0, 0.0,
        s, 0.0, c
    );
}





//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+10.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}