#include "fluid-system.h"

#include <chrono>
#include <cstdint>
#include <numbers>
#include <utility>
#include <execution>

#include "fluid-data.h"
#include "../../systems/system-structs.h"
#include "../../metrics/debug-stats.h"

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

    if (ctx.settings.sandbox.stepsPerTick != stepsPerTick_)
        stepsPerTick_ = ctx.settings.sandbox.stepsPerTick;

    if (ctx.settings.sandbox.particleCount != fluid_->GetParticleCount())
        fluid_->SetParticleCount(ctx.settings.sandbox.particleCount);

    if (ctx.settings.sandbox.particleSizeee != fluid_->GetParticleSize())
        fluid_->SetParticleSize(ctx.settings.sandbox.particleSizeee);
    
    if (ctx.settings.sandbox.particleSpacing != fluid_->GetParticleSpacing())
        fluid_->SetParticleSpacing(ctx.settings.sandbox.particleSpacing);
    
    if (ctx.settings.sandbox.smoothingRadius != smoothingRadius_)
        SetSmoothingRadius(ctx.settings.sandbox.smoothingRadius);

    if (ctx.settings.sandbox.targetDensity != targetDensity_)
        targetDensity_ = ctx.settings.sandbox.targetDensity;
    
    if (ctx.settings.sandbox.pressureMultiplier != pressureMultiplier_)
        pressureMultiplier_ = ctx.settings.sandbox.pressureMultiplier;
    
    if (ctx.settings.sandbox.viscosityStrength != viscosityStrength_)
        viscosityStrength_ = ctx.settings.sandbox.viscosityStrength;
    
    if (ctx.settings.sandbox.nearPressureMultiplier != nearPressureMultiplier_)
        nearPressureMultiplier_ = ctx.settings.sandbox.nearPressureMultiplier;

    if (ctx.settings.sandbox.boxWidth != boundingBox_.x)
        boundingBox_.x = ctx.settings.sandbox.boxWidth;

    if (ctx.settings.sandbox.boxHeight != boundingBox_.z)
        boundingBox_.z = ctx.settings.sandbox.boxHeight;

    if (ctx.settings.sandbox.boxDepth != boundingBox_.y)
        boundingBox_.y = ctx.settings.sandbox.boxDepth;

    if (!ctx.settings.sandbox.runSimulation)
        return;

    auto start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < ctx.settings.sandbox.stepsPerTick; ++i)
        RunSimulation(ctx.dT / ctx.settings.sandbox.stepsPerTick, ctx.settings.sandbox.gravity, ctx.settings.sandbox.restitution);

    auto t = std::chrono::steady_clock::now() - start;
    ctx.debugStats["fluid.sim_ms"].Add(std::chrono::duration<float, std::milli>(t).count());
}

void FluidSystem::RunSimulation(float dT, float gravity, float restitution)
{
// -------- Calculation --------

    for (auto& particle : fluid_->GetParticles())
    {
        if (gravity != 0.0f)
            particle.velocity.z -= gravity * dT;
        
        particle.predictedPosition = particle.position + particle.velocity * dT;
    }

    UpdateLookup(fluid_->GetParticles());

    densities_.resize(fluid_->GetParticleCount());
    nearDensities_.resize(fluid_->GetParticleCount());
    pressureTerms_.resize(fluid_->GetParticleCount());

    auto& particles = fluid_->GetParticles();

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        auto [density, nearDensity] = CalcDensities(particle.predictedPosition);
        densities_[particle.id] = density;
        nearDensities_[particle.id] = nearDensity;
        pressureTerms_[particle.id] = DensityToPressure(density) / (density * density);
    });

    accelerations_.resize(fluid_->GetParticleCount());

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        glm::vec3 acc = CalcPressure(particle);
        accelerations_[particle.id] = acc;
    });

// -------- Eggsecution --------

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& particle) {
        particle.velocity += accelerations_[particle.id] * dT;
        particle.position += particle.velocity * dT;
        CheckCollision(particle, restitution);
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

float FluidSystem::SmoothingFunction(float distance) const
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    return val * val * val * spikyScale_;
}

float FluidSystem::SmoothingDer(float distance) const
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    return -3.0f * val * val * spikyScale_;
}

std::tuple<float, float> FluidSystem::CalcDensities(glm::vec3 samplePos)
{
    float density = 0.0f;
    float nearDensity = 0.0f;

    GetPositionsInReach(samplePos, [&](uint32_t, const glm::vec3&, float d2) {
        float dist = std::sqrt(d2);
        density     += mass_ * SmoothingFunction(dist);
        nearDensity += mass_ * SmoothingFunctionNear(dist);
    });

    return std::tuple<float, float>(density, nearDensity);
}

float FluidSystem::SmoothingFunctionNear(float distance) const
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    return val * val * val * val * nearScale_;
}

float FluidSystem::SmoothingDerNear(float distance) const
{
    if (distance >= smoothingRadius_) return 0.0f;

    float val = std::max(0.0f, smoothingRadius_ - distance);
    return -4.0f * val * val * val * nearScale_;
}

float FluidSystem::ViscositySmoothing(float distance) const
{
    if (distance >= smoothingRadius_) return 0.0f;

    float t = sqrSmoothingRadius_ - distance * distance;
    return t * t * t * viscScale_;
}

float FluidSystem::DensityToPressure(float density)
{
    float dE = density - targetDensity_;
    float pressure = dE * pressureMultiplier_;
    return pressure;
}

glm::vec3 FluidSystem::CalcPressure(const ParticleData& pD)
{
    glm::vec3 acc(0.0f);
    const float selfTerm    = pressureTerms_[pD.id];
    const float selfNear    = nearDensities_[pD.id];
    const auto& particles   = fluid_->GetParticles();

    GetPositionsInReach(pD.predictedPosition, [&](uint32_t j, const glm::vec3& d, float d2) {
        if (j == pD.id) return;
        
        const auto& particle = particles[j];
        float dist = std::sqrt(d2);

        glm::vec3 dir;
        if (dist < 1e-3f)
        {
            auto min = std::min(pD.id, particle.id);
            auto max = std::max(pD.id, particle.id);

            uint32_t hash = min * 2654435761u ^ max;
            hash ^= hash >> 13;
            dir = glm::normalize(glm::vec3(
                float(hash & 0xFFFF) / 32768.0f - 1.0f,
                0.0f,//float((hash >> 16) & 0xFF) / 128.0f - 1.0f,
                float(hash % 977) / 488.5f - 1.0f));
            
            if (pD.id < particle.id) dir = -dir;
        }
        else
            dir = d / dist;
            
        float nearDensity = nearDensities_[particle.id];

        float t = pressureTerms_[j] + selfTerm;
        acc += dir * SmoothingDer(dist) * mass_ * t * 0.5f;
        
        float sharedNearPressure = (nearDensity + selfNear) * 0.5f * nearPressureMultiplier_;
        acc += sharedNearPressure * dir * SmoothingDerNear(dist) * mass_;

        float influence = ViscositySmoothing(std::sqrt(d2));
        acc += (particle.velocity - pD.velocity) * influence * viscosityStrength_;
    });

    return acc;
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

    std::sort(std::execution::par, lookupVector_.begin(), lookupVector_.end(),
        [](const auto& a, const auto& b) {
            return a.key < b.key;
        });

    std::for_each(std::execution::par, particles.begin(), particles.end(), [&](ParticleData& pD) {
        uint32_t key = lookupVector_[pD.id].key;
        uint32_t prevKey = pD.id == 0 ? UINT32_MAX : lookupVector_[pD.id - 1].key;

        if (key != prevKey)
            posInLookup_[key] = pD.id;
    });
}

glm::ivec3 FluidSystem::CellFromPosition(glm::vec3 pos) const
{
    return glm::ivec3(std::floor((pos.x / smoothingRadius_)), std::floor((pos.y / smoothingRadius_)), std::floor((pos.z / smoothingRadius_)));
}

uint32_t FluidSystem::HashCell(glm::ivec3 cell) const
{
    return (uint32_t)cell.x * 28579 + (uint32_t)cell.y * 58229 + (uint32_t)cell.z * 82759;
}

uint32_t FluidSystem::GetKey(uint32_t hash) const 
{
    return hash % (uint32_t)lookupVector_.size();
}

template <typename Fn>
void FluidSystem::GetPositionsInReach(glm::vec3 samplePos, Fn&& fn) const
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
            const float d2 = glm::dot(d, d);

            if (d2 < sqrSmoothingRadius_)
                fn(index, d, d2);
        }
    }
}

void FluidSystem::SetSmoothingRadius(float h)
{
    const float h2 = h * h;
    const float h5 = h2 * h2 * h;

    smoothingRadius_ = h;
    sqrSmoothingRadius_ = h2;

    spikyScale_ = 10.0f / (std::numbers::pi_v<float> * h5);
    nearScale_  = 15.0f / (std::numbers::pi_v<float> * h5 * h);
    viscScale_  = 10.0f / (std::numbers::pi_v<float> * h5 * h2 * h);
}
