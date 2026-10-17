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
        positionsBuffer_.SetUsage(GL_DYNAMIC_COPY);
        velocitiesBuffer_.SetUsage(GL_DYNAMIC_COPY);
        SetupParticles(particleCount);
    }
    ParticleData& GetParticleById(uint32_t id) { return particles_.at(id); };
    std::vector<ParticleData>& GetParticles() { return particles_; };
    const std::vector<ParticleData>& GetParticles() const { return particles_; };
    void SetParticleCount(uint32_t count) { SetupParticles(count); };
    void SetParticleSize(float particleSize) { particleSize_ = particleSize; ResetParticles(); };
    void SetParticleSpacing(float particleSpacing) { particleSpacing_ = particleSpacing; ResetParticles(); };
    uint32_t GetParticleCount() const { return static_cast<uint32_t>(particles_.size()); };
    float GetParticleSize() const { return particleSize_; };
    float GetParticleSpacing() const { return particleSpacing_; };
    void ResetParticles() { SetupParticles(particles_.size()); };
    bool GetIsFlat() const { return flat_; };
    void SetIsFlat(bool isFlat) { flat_ = isFlat; SetupParticles(particles_.size()); }
private:
    bool flat_ = true;
    float particleSize_, particleSpacing_;
    std::vector<ParticleData> particles_;
    void SetupParticles(uint32_t particleCount)
    {
        particles_.clear();
        particles_.reserve(particleCount);

        uint32_t n = 0;
        while ((flat_ ? n * n : n * n * n) < particleCount)
        {
            ++n;
        }

        const uint32_t layers = flat_ ? 1 : n;
        const float step = particleSize_ * particleSpacing_;
        const float offset = (n - 1) * step / 2;
        const float yOffset = flat_ ? 0.0f : offset;

        uint32_t id = 0;

        for (uint32_t i = 0; i < n; ++i)
        for (uint32_t j = 0; j < layers; ++j)
        for (uint32_t k = 0; k < n; ++k)
        {
            if (particles_.size() >= particleCount) break;

            glm::vec3 pos(k * step - offset, j * step - yOffset, i * step - offset);
            particles_.push_back({ .id = id,
                .position = pos,
                .predictedPosition = pos,
                .velocity = glm::vec3(0.0f) 
            });
            ++id;
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
