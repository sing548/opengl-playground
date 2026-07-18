#pragma once

#include <map>
#include <vector>
#include <memory>
#include <cstdint>

#include "../../../game/networking/network-bridge/shared-strucs.h"

struct SystemsContext;

class IGameplaySystem;

class ReplayDriver
{
public:
    ReplayDriver(const std::vector<std::unique_ptr<IGameplaySystem>>& systems, float fixedDelta)
        : systems_(systems), fixedDelta_(fixedDelta) {}
    void Run(const SystemsContext& originalCtx, const std::map<uint32_t, InputState>& stateToReplay);
private:
    float fixedDelta_;
    const std::vector<std::unique_ptr<IGameplaySystem>>& systems_;

    std::unordered_map<uint32_t, InputState> replayCurrent_;
    std::unordered_map<uint32_t, InputState> replayPrevious_;
};
