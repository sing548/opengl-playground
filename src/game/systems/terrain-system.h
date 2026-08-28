#ifndef TERRAIN_SYSTEM_H
#define TERRAIN_SYSTEM_H

#include "../rendering/terrain/flat-worlds/terrain-handler.h"
#include "../../engine/systems/i-gameplay-system.h"

#include "system-order.h"

struct SystemsContext;

class TerrainSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::TerrainSystem); }
private:
};

#endif
