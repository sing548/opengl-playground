#pragma once

#include <glm/glm.hpp>

#include "../../gpu/gpu-buffer.h"
#include <iostream>

struct ParticleData {
    uint32_t id;
    glm::vec3 position;
    glm::vec3 predictedPosition;
    glm::vec3 velocity;
};

class FluidData
{
public:
    FluidData(uint32_t particleCount, float particleSize, float particleSpacing) : particleSize_(particleSize), particleSpacing_(particleSpacing)
    {
        SetupParticles(particleCount);
    }
    ParticleData& GetParticleById(uint32_t id) { return particles_.at(id); };
    std::vector<ParticleData>& GetParticles() { return particles_; };
    const std::vector<ParticleData>& GetParticles() const { return particles_; };
    void SetParticleCount(uint32_t count) { SetupParticles(count); };
    void SetParticleSize(float particleSize) { particleSize_ = particleSize; ResetParticles(); };
    void SetParticleSpacing(float particleSpacing) { particleSpacing_ = particleSpacing; ResetParticles(); };
    int GetParticleCount() const { return particles_.size(); };
    float GetParticleSize() const { return particleSize_; };
    float GetParticleSpacing() const { return particleSpacing_; };
    void ResetParticles() { SetupParticles(particles_.size()); };
private:
    float particleSize_, particleSpacing_;
    std::vector<ParticleData> particles_;
    void SetupParticles(uint32_t particleCount)
    {
        particles_.clear();

        auto sq = std::sqrt(particleCount);
        int max = (int)sq;

        float xOffset = (max - 1) * particleSpacing_ * particleSize_ / 2;

        if (sq > max)
            ++max;

        float zOffset = (max - 1) * particleSpacing_ * particleSize_ / 2;

        uint32_t id = 0;
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

        UploadToGPU();
    }

//------------- GPU -------------
public:
    void UploadToGPU()
    {
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocities;

        positions.reserve(particles_.size());
        velocities.reserve(particles_.size());

        for (auto& el : particles_)
        {
            positions.push_back(glm::vec4(el.position, 0.0f));
            velocities.push_back(glm::vec4(el.velocity, 0.0f));
        }

        positionsBuffer_.Upload(positions);
        velocitiesBuffer_.Upload(velocities);

        #ifndef NDEBUG
            VerifyUpload();
        #endif
    }

    bool OnGPU() const { return onGPU_; }
    void SetOnGPU(bool b) 
    { 
        bool reset = b != onGPU_;

        onGPU_ = b;
        
        if (reset)
            ResetParticles();
    }

    GPUBuffer& GetPositionsBuffer() { return positionsBuffer_; }
    const GPUBuffer& GetPositionsBuffer() const { return positionsBuffer_; }
    GPUBuffer& GetVelocitiesBuffer() { return velocitiesBuffer_; }
    const GPUBuffer& GetVelocitiesBuffer() const { return velocitiesBuffer_; }
private:
    bool onGPU_ = false;
    GPUBuffer positionsBuffer_;
    GPUBuffer velocitiesBuffer_;

    void VerifyUpload() const
    {
        std::vector<glm::vec4> gpu;
        positionsBuffer_.ReadData(gpu);

        if (gpu.size() != particles_.size())
        {
            std::cout << "Error reading GPU particle positions - read amount: " << gpu.size()
                << ", real amount: " << particles_.size() << std::endl;
            return;
        }

        for (size_t i = 0; i < gpu.size(); ++i)
        {
            if (glm::vec3(gpu[i]) != particles_[i].position || gpu[i].w != 0.0f)
            {
                auto& part = particles_[i].position;
                std::cout << "Error reading GPU particle position - read value - i: " << i
                    << "[" << gpu[i].x << ", " << gpu[i].y << ", " << gpu[i].z << ", " << gpu[i].w << "]"
                    << ", real value: " << "[" << part.x << ", " << part.y << ", " << part.z << "]"
                    << std::endl;
                return;
            }
        }

        gpu.clear();
        velocitiesBuffer_.ReadData(gpu);

        if (gpu.size() != particles_.size())
        {
            std::cout << "Error reading GPU particle velocities - read amount: " << gpu.size()
                << ", real amount: " << particles_.size() << std::endl;
            return;
        }

        for (size_t i = 0; i < gpu.size(); ++i)
        {
            if (glm::vec3(gpu[i]) != particles_[i].velocity || gpu[i].w != 0.0f)
            {
                auto& part = particles_[i].velocity;
                std::cout << "Error reading GPU particle velocity - read value - i: " << i
                    << "[" << gpu[i].x << ", " << gpu[i].y << ", " << gpu[i].z << ", " << gpu[i].w << "]"
                    << ", real value: " << "[" << part.x << ", " << part.y << ", " << part.z << "]"
                    << std::endl;
                return;
            }
        }
    }
};
