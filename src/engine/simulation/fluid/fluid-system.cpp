#include "fluid-system.h"

#include <chrono>
#include <cstdint>
#include <numbers>
#include <utility>
#include <execution>

#include "fluid-data.h"
#include "../../helpers/file-helper.h"
#include "../../metrics/debug-stats.h"
#include "../../systems/system-structs.h"

FluidSystem::FluidSystem() :
    predictionShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-prediction.comp"),
    countShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-count.comp"),
    scanShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-scan.comp"),
    scatterShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-scatter.comp"),
    densityShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-densities.comp"),
    forcesShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-forces.comp"),
    integrationShader_(std::filesystem::path(FileHelper::GetShaderDir()) / "game" / "compute" / "fluid-integration.comp")
{
    fluid_ = std::make_unique<FluidData>(25, 0.5f, 1.0f);

    gPredicted_.SetUsage(GL_DYNAMIC_COPY);
    gDensities_.SetUsage(GL_DYNAMIC_COPY);
    gAccelerations_.SetUsage(GL_DYNAMIC_COPY);
    gHashes_.SetUsage(GL_DYNAMIC_COPY);
    gCounts_.SetUsage(GL_DYNAMIC_COPY);
    gStarts_.SetUsage(GL_DYNAMIC_COPY);
    gCursor_.SetUsage(GL_DYNAMIC_COPY);
    gPositionsSpare_.SetUsage(GL_DYNAMIC_COPY); 
    gVelocitiesSpare_.SetUsage(GL_DYNAMIC_COPY);
    gPredictedSpare_.SetUsage(GL_DYNAMIC_COPY);     
    gHashesSpare_.SetUsage(GL_DYNAMIC_COPY);    

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

    if (static_cast<uint32_t>(ctx.settings.sandbox.particleCount) != fluid_->GetParticleCount())
        fluid_->SetParticleCount(static_cast<uint32_t>(ctx.settings.sandbox.particleCount));

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
    
    if (ctx.settings.sandbox.gpuSimulation != fluid_->OnGPU())
        fluid_->SetOnGPU(ctx.settings.sandbox.gpuSimulation);

    if (ctx.settings.sandbox.flatSimulation != fluid_->GetIsFlat())
    {
        fluid_->SetIsFlat(ctx.settings.sandbox.flatSimulation);
        SetSmoothingRadius(smoothingRadius_);
        SetDefaultValues(ctx);
    }

    if (!ctx.settings.sandbox.runSimulation && !ctx.settings.sandbox.singleStep)
        return;

    auto start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < ctx.settings.sandbox.stepsPerTick; ++i)
    {
        !fluid_->OnGPU() ?
            RunSimulation(ctx.dT / ctx.settings.sandbox.stepsPerTick, ctx.settings.sandbox.gravity, ctx.settings.sandbox.restitution) :
            RunSimulationGPU(ctx.dT / ctx.settings.sandbox.stepsPerTick, ctx.settings.sandbox.gravity, ctx.settings.sandbox.restitution);
        
        if (!ctx.settings.sandbox.runSimulation && ctx.settings.sandbox.singleStep)
        {
            ctx.settings.sandbox.singleStep = false;
            break;
        }
    }

    auto t = std::chrono::steady_clock::now() - start;
    ctx.debugStats["fluid.sim_ms"].Add(std::chrono::duration<float, std::milli>(t).count());
}

void FluidSystem::SetDefaultValues(SystemsContext& ctx)
{
    if (fluid_->GetIsFlat())
    {
        ctx.settings.sandbox.boxWidth = 25.0f;
        ctx.settings.sandbox.boxDepth = 20.0f;
        ctx.settings.sandbox.boxHeight = 15.0f;
        ctx.settings.sandbox.particleCount = 3000;
        ctx.settings.sandbox.smoothingRadius = 1.5f;
        ctx.settings.sandbox.targetDensity = 6.5f;
        ctx.settings.sandbox.pressureMultiplier = 6000.0f;
        ctx.settings.sandbox.nearPressureMultiplier = 3.0f;
        ctx.settings.sandbox.viscosityStrength = 0.1f;
        ctx.settings.sandbox.stepsPerTick = 3;
        ctx.settings.sandbox.gravity = 0.0f;
        ctx.settings.sandbox.restitution = 0.2f;
    }
    else
    {
        ctx.settings.sandbox.boxWidth = 8.0f;
        ctx.settings.sandbox.boxDepth = 6.0f;
        ctx.settings.sandbox.boxHeight = 5.0f;
        ctx.settings.sandbox.particleCount = 8000;
        ctx.settings.sandbox.smoothingRadius = 1.0f;
        ctx.settings.sandbox.targetDensity = 17.1f;
        ctx.settings.sandbox.pressureMultiplier = 3000.0f;
        ctx.settings.sandbox.nearPressureMultiplier = 0.2f;
        ctx.settings.sandbox.viscosityStrength = 0.1f;
        ctx.settings.sandbox.stepsPerTick = 3;
        ctx.settings.sandbox.gravity = 0.0f;
        ctx.settings.sandbox.restitution = 0.2f;
    }
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

void FluidSystem::RunSimulationGPU(float dT, float gravity, float restitution)
{
    uint32_t count = fluid_->GetParticleCount();

    uint32_t tableSize = 2 * count;

    gPredicted_.Resize(count * sizeof(glm::vec4));
    gHashes_.Resize(count * sizeof(uint32_t));
    gCounts_.Resize(tableSize * sizeof(uint32_t));
    gCursor_.Resize(tableSize * sizeof(uint32_t));
    gStarts_.Resize((tableSize + 1) * sizeof(uint32_t));
    gDensities_.Resize(count * sizeof(glm::vec4));
    gAccelerations_.Resize(count * sizeof(glm::vec4));
    gPositionsSpare_.Resize(count * sizeof(glm::vec4)); 
    gVelocitiesSpare_.Resize(count * sizeof(glm::vec4));
    gPredictedSpare_.Resize(count * sizeof(glm::vec4)); 
    gHashesSpare_.Resize(count * sizeof(uint32_t));    

    fluid_->GetPositionsBuffer().BindBase(0);
    fluid_->GetVelocitiesBuffer().BindBase(1);
    gPredicted_.BindBase(2);
    gDensities_.BindBase(3);
    gAccelerations_.BindBase(4);
    gHashes_.BindBase(5);
    gCounts_.BindBase(6);
    gStarts_.BindBase(7);
    gCursor_.BindBase(8);
    gPositionsSpare_.BindBase(9); 
    gVelocitiesSpare_.BindBase(10);
    gPredictedSpare_.BindBase(11); 
    gHashesSpare_.BindBase(12);    

    // -------- Prediction -------- //
    predictionShader_.SetFloat("dT", dT);
    predictionShader_.SetFloat("gravity", gravity);
    predictionShader_.SetUInt("count", count);
    
    predictionShader_.DispatchFor(count);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // -------- Count -------- //
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    gCounts_.Clear();

    countShader_.SetUInt("count", count);
    countShader_.SetUInt("tableSize", tableSize);
    countShader_.SetFloat("inversCellSize", 1 / smoothingRadius_);

    countShader_.DispatchFor(count);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // -------- Count --------
    scanShader_.SetUInt("tableSize", tableSize);

    scanShader_.Dispatch(1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // -------- Scatter --------
    scatterShader_.SetUInt("count", count);
    scatterShader_.SetUInt("tableSize", tableSize);

    scatterShader_.DispatchFor(count);

    std::swap(fluid_->GetPositionsBuffer(), gPositionsSpare_);
    std::swap(fluid_->GetVelocitiesBuffer(), gVelocitiesSpare_);
    std::swap(gPredicted_, gPredictedSpare_);
    std::swap(gHashes_, gHashesSpare_);

    fluid_->GetPositionsBuffer().BindBase(0);
    fluid_->GetVelocitiesBuffer().BindBase(1);
    gPredicted_.BindBase(2);
    gHashes_.BindBase(5);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // -------- Density -------- //
    densityShader_.SetUInt("count", count);
    densityShader_.SetUInt("tableSize", tableSize);
    densityShader_.SetFloat("mass", mass_);
    densityShader_.SetFloat("nearScale", nearScale_);
    densityShader_.SetFloat("spikyScale", spikyScale_);
    densityShader_.SetFloat("targetDensity", targetDensity_);
    densityShader_.SetFloat("smoothingRadius", smoothingRadius_);
    densityShader_.SetFloat("inversCellSize", 1 / smoothingRadius_);
    densityShader_.SetFloat("pressureMultiplier", pressureMultiplier_);


    densityShader_.DispatchFor(count);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // -------- Forces -------- //
    forcesShader_.SetUInt("count", count);
    forcesShader_.SetBool("isFlat", fluid_->GetIsFlat());
    forcesShader_.SetUInt("tableSize", tableSize);
    forcesShader_.SetFloat("mass", mass_);
    forcesShader_.SetFloat("nearScale", nearScale_);
    forcesShader_.SetFloat("viscScale", viscScale_);
    forcesShader_.SetFloat("spikyScale", spikyScale_);
    forcesShader_.SetFloat("smoothingRadius", smoothingRadius_);
    forcesShader_.SetFloat("inversCellSize", 1 / smoothingRadius_);
    forcesShader_.SetFloat("viscoscityStrength", viscosityStrength_);
    forcesShader_.SetFloat("sqrSmoothingRadius", sqrSmoothingRadius_);
    forcesShader_.SetFloat("nearPressureMultiplier", nearPressureMultiplier_);

    forcesShader_.DispatchFor(count);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // -------- Eggsecution -------- //
    integrationShader_.SetFloat("dT", dT);
    integrationShader_.SetFloat("restitution", restitution);
    integrationShader_.SetUInt("count", count);
    integrationShader_.SetVec3("boundingBox", boundingBox_);
    
    integrationShader_.DispatchFor(count);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void FluidSystem::CheckCollision(ParticleData& particle, float restitution)
{
    if (std::abs(particle.position.x) > boundingBox_.x)
    {
        particle.velocity.x *= -restitution;
        particle.position.x = glm::clamp(particle.position.x, -boundingBox_.x, boundingBox_.x);
    }

    if (std::abs(particle.position.y) > boundingBox_.y)
    {
        particle.velocity.y *= -restitution;
        particle.position.y = glm::clamp(particle.position.y, -boundingBox_.y, boundingBox_.y);
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
                fluid_->GetIsFlat() ? 0.0f : float((hash >> 16) & 0xFF) / 128.0f - 1.0f,
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

        float influence = ViscositySmoothing(dist);
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
    const float h6 = h2 * h2 * h2;
    const float h7 = h6 * h;
    const float h9 = h7 * h2;

    smoothingRadius_ = h;
    sqrSmoothingRadius_ = h2;

    if (fluid_->GetIsFlat())
    {
        spikyScale_ = 10.0f / (std::numbers::pi_v<float> * h5);
        nearScale_  = 15.0f / (std::numbers::pi_v<float> * h5 * h);
        viscScale_  = 10.0f / (std::numbers::pi_v<float> * h5 * h2 * h);
    }
    else 
    {
        spikyScale_ = 15.0f  / (std::numbers::pi_v<float> * h6);
        nearScale_  = 105.0f / (4 * std::numbers::pi_v<float> * h7);
        viscScale_  = 315.0f  / (64 * std::numbers::pi_v<float> * h9);
    }
}
