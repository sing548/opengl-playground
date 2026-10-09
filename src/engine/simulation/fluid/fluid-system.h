#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "../../systems/i-gameplay-system.h"

#include "../../../game/systems/system-order.h"

class FluidData;

struct ParticleData;
struct SystemsContext;

class FluidSystem : public IGameplaySystem
{
public:
    FluidSystem();
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    bool CanReplay() override { return false; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::FluidSystem); }

    const FluidData& GetFluid() const { return *fluid_; };
private:
    const float mass_             = 1.0f;
    int stepsPerTick_             = 0;
    float targetDensity_          = 0.0f;
    float smoothingRadius_        = 0.0f;
    float viscosityStrength_      = 0.0f;
    float sqrSmoothingRadius_     = 0.0f;
    float pressureMultiplier_     = 0.0f;
    float nearPressureMultiplier_ = 0.0f;

    float spikyScale_   = 0.0f;
    float nearScale_    = 0.0f;
    float viscScale_    = 0.0f;

    std::vector<glm::ivec3> offsets_;

    std::vector<uint32_t> posInLookup_;

    struct LookupEntry {
        int index;
        uint32_t key;
        uint32_t hash;
    };
    std::vector<LookupEntry> lookupVector_;

    std::vector<float> densities_;
    std::vector<float> nearDensities_;
    std::vector<float> pressureTerms_;
    std::vector<glm::vec3> accelerations_;

    glm::vec3 boundingBox_ = { 20.0f, 10.0f, 15.0f };

    std::unique_ptr<FluidData> fluid_;

    void RunSimulation(float dT, float gravity, float restitution);

    void CheckCollision(ParticleData& particle, float restitution);

    float SmoothingFunction(float distance) const;
    float SmoothingDer(float distance) const;
    
    float SmoothingFunctionNear(float distance) const;
    float SmoothingDerNear(float distance) const;
    
    std::tuple<float, float> CalcDensities(glm::vec3 samplePos);

    float ViscositySmoothing(float distance) const;
    
    float DensityToPressure(float density);
    glm::vec3 CalcPressure(const ParticleData& pD);

    void UpdateLookup(std::vector<ParticleData>& positions);
    glm::ivec3 CellFromPosition(glm::vec3 pos) const;
    uint32_t HashCell(glm::ivec3 cell) const;
    uint32_t GetKey(uint32_t hash) const;
    //void GetPositionsInReach(glm::vec3 samplePos, std::function<void(uint32_t)> callback);

    template <typename Fn>
    void GetPositionsInReach(glm::vec3 samplePos, Fn&& fn) const;

    void SetSmoothingRadius(float h);
};
