#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

class GameWorld;
class AssetManager;
class InputState;

struct SystemsContext
{
    float dT;
    GameWorld& world;
    AssetManager& assMan;
    std::unordered_map<uint32_t, InputState> current;
    std::unordered_map<uint32_t, InputState> previous;
    uint32_t localPlayerId;
    bool authoritative;
    bool replay;
    const std::unordered_map<std::string, bool>&  settings;
};
