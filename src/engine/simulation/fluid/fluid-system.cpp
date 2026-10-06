#include "fluid-system.h"

#include <cstdint>
#include <numbers>
#include <utility>
#include <execution>

#include "fluid-data.h"
#include "../../systems/system-structs.h"

FluidSystem::FluidSystem()
{
    fluid_ = std::make_unique<FluidData>(25, 0.5f, 1.0f);

    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    for (int z = -1; z <= 1; ++z)
        offsets_.push_back({ x, y, z});
}

// https://lucasschuermann.com/writing/implementing-sph-in-2d

void FluidSystem::Update(SystemsContext& ctx)
{

// -------- Settings --------

    if (ctx.settings.sandbox.particleCount != fluid_->GetParticleCount())
        fluid_->SetParticleCount(ctx.settings.sandbox.particleCount);

    if (ctx.settings.sandbox.particleSize != fluid_->GetParticleSize())
        fluid_->SetParticleSize(ctx.settings.sandbox.particleSize);
    
    if (ctx.settings.sandbox.particleSpacing != fluid_->GetParticleSpacing())
        fluid_->SetParticleSpacing(ctx.settings.sandbox.particleSpacing);
    
    if (ctx.settings.sandbox.smoothingRadius != smoothingRadius_)
    {
        smoothingRadius_ = ctx.settings.sandbox.smoothingRadius;
        sqrSmoothingRadius_ = smoothingRadius_ * smoothingRadius_;
    }

    if (ctx.settings.sandbox.targetDensity != targetDensity_)
        targetDensity_ = ctx.settings.sandbox.targetDensity;
    
    if (ctx.settings.sandbox.pressureMultiplier != pressureMultiplier_)
        pressureMultiplier_ = ctx.settings.sandbox.pressureMultiplier;
    
    if (ctx.settings.sandbox.viscosityStrength != viscosityStrength_)
        viscosityStrength_ = ctx.settings.sandbox.viscosityStrength;
    
    if (ctx.settings.sandbox.nearPressureMultiplier != nearPressureMultiplier_)
        nearPressureMultiplier_ = ctx.settings.sandbox.nearPressureMultiplier;

    if (!ctx.settings.sandbox.runSimulation)
        return;

// -------- Calculation --------

    for (auto& particle : fluid_->GetParticles())
    {
        if (ctx.settings.sandbox.gravity != 0.0f)
            particle.velocity.z -= ctx.settings.sandbox.gravity * ctx.dT;
        
        particle.predictedPosition = particle.position + particle.velocity * ctx.dT;
    }

    UpdateLookup(fluid_->GetParticles());

    densities_.resize(fluid_->GetParticleCount());
    nearDensities_.resize(fluid_->GetParticleCount());

    auto& particles = fluid_->GetParticles();

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        densities_[particle.id] = CalcDensity(particle.predictedPosition);
        nearDensities_[particle.id] = CalcNearDensity(particle.predictedPosition);
    });

    accelerations_.resize(fluid_->GetParticleCount());

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        glm::vec3 acc = CalcPressure(particle);
        acc += CalcViscosity(particle);
        accelerations_[particle.id] = acc;
    });

// -------- Eggsecution --------

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        particle.velocity += accelerations_[particle.id] * ctx.dT;
        particle.position += particle.velocity * ctx.dT;
        CheckCollision(particle, ctx.settings.sandbox.restitution);
    });
}

void FluidSystem::CheckCollision(ParticleData& particle, float restitution)
{
    if (std::abs(particle.position.x) > boundingBox_.x)
    {
        particle.velocity.x *= -restitution;
        particle.position.x = glm::clamp(particle.position.x, -boundingBox_.x, boundingBox_.x);
    }

    if (std::abs(particle.position.z) > boundingBox_.z)
    {
        particle.velocity.z *= -restitution;
        particle.position.z = glm::clamp(particle.position.z, -boundingBox_.z, boundingBox_.z);
    }
}

float FluidSystem::SmoothingFunction(float distance)
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    float vol = std::numbers::pi * std::pow(smoothingRadius_, 5) / 10;
    return std::pow(val, 3) / vol;
}

float FluidSystem::SmoothingDer(float distance)
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    float vol = std::numbers::pi * std::pow(smoothingRadius_, 5) / 10;
    return -3.0f * std::pow(val, 2) / vol;
}

float FluidSystem::CalcDensity(glm::vec3 samplePos)
{
    float density = 0.0f;
    const auto& particles = fluid_->GetParticles();

    GetPositionsInReach(samplePos, [&](uint32_t j) {
        float dist = glm::length(particles[j].predictedPosition - samplePos);
        density += mass_ * SmoothingFunction(dist);
    });

    return density;
}

float FluidSystem::SmoothingFunctionNear(float distance)
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    float vol = std::numbers::pi * std::pow(smoothingRadius_, 6) / 15;
    return std::pow(val, 4) / vol;
}

float FluidSystem::SmoothingDerNear(float distance)
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    float vol = std::numbers::pi * std::pow(smoothingRadius_, 6) / 15;
    return -4.0f * std::pow(val, 3) / vol;
}

float FluidSystem::CalcNearDensity(glm::vec3 samplePos)
{
    float density = 0.0f;
    const auto& particles = fluid_->GetParticles();

    GetPositionsInReach(samplePos, [&](uint32_t j) {
        float dist = glm::length(particles[j].predictedPosition - samplePos);
        density += mass_ * SmoothingFunctionNear(dist);
    });

    return density;
}

float FluidSystem::ViscositySmoothing(float distance)
{
    if (distance >= smoothingRadius_) return 0.0f;

    return 4 / (std::numbers::pi * std::pow(smoothingRadius_, 8)) * std::pow(std::pow(smoothingRadius_, 2) - std::pow(distance, 2), 3);
}

float FluidSystem::DensityToPressure(float density)
{
    float dE = density - targetDensity_;
    float pressure = dE * pressureMultiplier_;
    return pressure;
}

glm::vec3 FluidSystem::CalcPressure(const ParticleData& pD)
{
    glm::vec3 gradient(0.0f);
    const float selfDensity = densities_.at(pD.id);
    const float selfNear    = nearDensities_.at(pD.id);

    GetPositionsInReach(pD.predictedPosition, [&](uint32_t j) {
        if (j == pD.id) return;
        
        const auto& particle = fluid_->GetParticleById(j);
        float dist = glm::length(particle.predictedPosition - pD.predictedPosition);

        glm::vec3 dir;
        if (dist < 1e-3f)
        {
            auto min = std::min(pD.id, particle.id);
            auto max = std::min(pD.id, particle.id);

            uint32_t hash = min * 2654435761u ^ max;
            hash ^= hash >> 13;
            dir = glm::normalize(glm::vec3(
                float(hash & 0xFFFF) / 32768.0f - 1.0f,
                0.0f,//float((hash >> 16) & 0xFF) / 128.0f - 1.0f,
                float(hash % 977) / 488.5f - 1.0f));
            
            if (pD.id < particle.id) dir = -dir;
        }
        else
            dir = (particle.predictedPosition - pD.predictedPosition) / dist;
            
        float density = densities_.at(particle.id);
        float nearDensity = nearDensities_.at(particle.id);

        float t = DensityToPressure(density) / (density * density)
                  + DensityToPressure(selfDensity) / (selfDensity * selfDensity);
        gradient += dir * SmoothingDer(dist) * mass_ * t * 0.5f;
        
        float sharedNearPressure = (nearDensity + selfNear) * 0.5f * nearPressureMultiplier_;
        gradient += sharedNearPressure * dir * SmoothingDerNear(dist) * mass_;
    });

    return gradient;
}

glm::vec3 FluidSystem::CalcViscosity(const ParticleData& pD)
{
    glm::vec3 viscosity(0.0f);
    
    GetPositionsInReach(pD.predictedPosition, [&](uint32_t j) {
        if (j != pD.id)
        {
            const ParticleData& otherParticle = fluid_->GetParticleById(j);
            float distance = glm::length(pD.predictedPosition - otherParticle.predictedPosition);
            float influence = ViscositySmoothing(distance);
            viscosity += (otherParticle.velocity - pD.velocity) * influence;
        }
    });

    return viscosity * viscosityStrength_;
}

void FluidSystem::UpdateLookup(std::vector<ParticleData>& particles)
{
    lookupVector_.resize(particles.size());
    posInLookup_.resize(particles.size());

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& pD) {
        auto cell = CellFromPosition(pD.predictedPosition);
        uint32_t hash = HashCell(cell);
        uint32_t key = GetKey(hash);
        lookupVector_[pD.id] = LookupEntry(pD.id, key, hash);
        posInLookup_[pD.id] = UINT32_MAX;
    });

    std::sort(lookupVector_.begin(), lookupVector_.end(),
        [](const auto& a, const auto& b) {
            return a.key < b.key;
        });

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& pD) {
        uint32_t key = lookupVector_.at(pD.id).key;
        uint32_t prevKey = pD.id == 0 ? UINT32_MAX : lookupVector_.at(pD.id - 1).key;

        if (key != prevKey)
            posInLookup_[key] = pD.id;
    });
}

glm::ivec3 FluidSystem::CellFromPosition(glm::vec3 pos)
{
    return glm::ivec3((int)(pos.x / smoothingRadius_), (int)(pos.y / smoothingRadius_), (int)(pos.z / smoothingRadius_));
}

uint32_t FluidSystem::HashCell(glm::ivec3 cell)
{
    return (uint32_t)cell.x * 28579 + (uint32_t)cell.y * 58229 + (uint32_t)cell.z * 82759;
}

uint32_t FluidSystem::GetKey(uint32_t hash)
{
    return hash % (uint32_t)lookupVector_.size();
}

void FluidSystem::GetPositionsInReach(glm::vec3 samplePos, std::function<void(uint32_t)> callback)
{
    constexpr uint32_t empty = UINT32_MAX;

    auto center = CellFromPosition(samplePos);
    const auto& particles = fluid_->GetParticles();

    for (const auto& offset : offsets_)
    {
        uint32_t cellHash = HashCell({ center.x + offset.x, center.y + offset.y, center.z + offset.z });
        uint32_t key = GetKey(cellHash);
        uint32_t firstPosInLookup = posInLookup_[key];

        if (firstPosInLookup == empty) continue;

        for (size_t i = firstPosInLookup; i < lookupVector_.size(); ++i)
        {
            const LookupEntry& entry = lookupVector_[i];
            
            if (entry.key != key) break;
            if (entry.hash != cellHash) continue;
            
            uint32_t index = entry.index;
            const glm::vec3 d = particles[index].predictedPosition - samplePos;

            if (glm::dot(d, d) < sqrSmoothingRadius_)
                callback(index);
        }
    }
}
