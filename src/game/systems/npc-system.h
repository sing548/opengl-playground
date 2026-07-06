#ifndef NPC_SYSTEM_H
#define NPC_SYSTEM_H

#include "../../engine/systems/i-gameplay-system.h"

#include "system-order.h"

struct SystemsContext;

class GameWorld;
class AssetManager;

class NpcSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& context) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    int GetOrder() const override { return static_cast<int>(SystemOrder::NpcSystem); }
private:
};

#endif
