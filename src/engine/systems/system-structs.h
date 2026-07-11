#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

#include "../settings.h"

class Window;
class GameWorld;
class InputState;
class AssetManager;
class NetworkBridge;
class ITerrainHandler;

struct SystemsContext
{
    float dT;
    Window& window;
    GameWorld& world;
    AssetManager& assMan;
    NetworkBridge& bridge;
    ITerrainHandler& terrainHandler;
    std::unordered_map<uint32_t, InputState>& current;
    std::unordered_map<uint32_t, InputState>& previous;
    uint32_t localPlayerId;
    bool authoritative;
    bool replay;
    float alpha;
    Settings&  settings;
};

enum class GameplayPhase
{
    Input, PreSimulation, Simulation, PostSimulation, PreRender, PostRender, UNDEFINED
};
