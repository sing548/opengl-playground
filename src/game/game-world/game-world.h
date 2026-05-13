#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <unordered_map>
#include <cstdint>

#include "../../engine/models/scene.h"

struct ShotData
{
    uint32_t id;
};

struct PlayerData
{
    float lastHit = 0;
    int lifes = 10;
};

struct NpcData
{
    float lastShot = 0;
    float lastHit = 0;
    int lifes = 10;
};

struct RemovedEntities
{
    std::vector<uint32_t> all;
    std::vector<uint32_t> npcs;
    std::vector<uint32_t> shots;
    std::vector<uint32_t> players;
};

class GameWorld {
public:
    auto& GetScene() { return scene_;};
    const auto& GetScene() const { return scene_; };

    void MarkEntityForDelete(uint32_t id);
    RemovedEntities RemoveMarkedEntities();

    // NOTE: This won't scale indefinetely. Once it grows, switch pattern
    auto& GetPlayerData(uint32_t id) { return playerData_.at(id); };
    const auto& GetPlayerData(uint32_t id) const { return playerData_.at(id); };
    auto& GetPlayerData() { return playerData_; };
    const auto& GetPlayerData() const { return playerData_; };

    auto& GetNpcData(uint32_t id) { return npcData_.at(id); };
    const auto& GetNpcData(uint32_t id) const { return npcData_.at(id); };
    auto& GetNpcData() { return npcData_; };
    const auto& GetNpcData() const { return npcData_; };

    auto& GetShotData(uint32_t id) { return shotData_.at(id); };
    const auto& GetShotData(uint32_t id) const { return shotData_.at(id); };
    auto& GetShotData() { return shotData_; };
    const auto& GetShotData() const { return shotData_; };

    uint32_t AddPlayer(uint32_t id, PlayerData playerData);
    uint32_t AddNpc(uint32_t id);
    uint32_t AddShot(uint32_t id);
    void     RemoveEntity(uint32_t id);

    bool IsPlayer(uint32_t id) const { return playerData_.contains(id); };
    bool IsNpc(uint32_t id) const { return npcData_.contains(id); };
    bool IsShot(uint32_t id) const { return shotData_.contains(id); };

private:
    Scene scene_;
    std::unordered_map<uint32_t, NpcData> npcData_;
    std::unordered_map<uint32_t, ShotData> shotData_;
    std::unordered_map<uint32_t, PlayerData> playerData_;
};

#endif
