#pragma once

#include <glm/glm.hpp>

struct ParticleData {
    uint16_t id;
    glm::vec3 position;
    glm::vec3 predictedPosition;
    glm::vec3 velocity;
};


class FluidData
{
public:
    FluidData(uint16_t particleCount, float particleSize, float particleSpacing) : particleSize_(particleSize), particleSpacing_(particleSpacing)
    {
        SetupParticles(particleCount);
    }
    ParticleData& GetParticleById(uint16_t id) { return particles_.at(id); };
    std::vector<ParticleData>& GetParticles() { return particles_; };
    const std::vector<ParticleData>& GetParticles() const { return particles_; };
    void SetParticleCount(uint16_t count) { SetupParticles(count); };
    void SetParticleSize(float particleSize) { particleSize_ = particleSize; ResetParticles(); };
    void SetParticleSpacing(float particleSpacing) { particleSpacing_ = particleSpacing; ResetParticles(); };
    int GetParticleCount() const { return particles_.size(); };
    float GetParticleSize() const { return particleSize_; };
    float GetParticleSpacing() const { return particleSpacing_; };
    void ResetParticles() { SetupParticles(particles_.size()); };
private:
    float particleSize_, particleSpacing_;
    std::vector<ParticleData> particles_;
    void SetupParticles(uint16_t particleCount)
    {
        particles_.clear();

        auto sq = std::sqrt(particleCount);
        int max = (int)sq;

        float xOffset = (max - 1) * particleSpacing_ * particleSize_ / 2;

        if (sq > max)
            ++max;

        float zOffset = (max - 1) * particleSpacing_ * particleSize_ / 2;

        uint16_t id = 0;
        for (int i = 0; i < (int)max; ++i)
        {
            for (int j = 0; j < (int)max && particles_.size() < particleCount; ++j)
            {
                ParticleData p { .id = id,
                                 .position = glm::vec3((float)(particleSpacing_ * j * particleSize_) - xOffset, 0.0f, (float)(particleSpacing_ * i * particleSize_) - zOffset),
                                 .predictedPosition = glm::vec3((float)(particleSpacing_ * j * particleSize_) - xOffset, 0.0f, (float)(particleSpacing_ * i * particleSize_) - zOffset),
                                 .velocity = glm::vec3(0.0f) };
                particles_.push_back(p);
                ++id;
            }
        }
    }
};
