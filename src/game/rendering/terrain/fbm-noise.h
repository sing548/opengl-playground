#ifndef FBM_NOISE_H
#define FBM_NOISE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

// Fractal Brownian Motion (FBM) - https://thebookofshaders.com/13/

class FBMNoise
{
public:
    static float GenNoise(int octaves, float lacunarity, float gain, glm::vec2 pos);
private:
    static float snoise(glm::vec2 v);
    static glm::vec3 mod289(glm::vec3 x);
    static glm::vec2 mod289(glm::vec2 x);
    static glm::vec3 permute(glm::vec3 x);
};

#endif
