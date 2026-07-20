#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <cstdint>
#include <unordered_map>

#include "../../engine/models/scene.h"
#include <utility>

struct ShotData
{
    uint32_t id;
    uint32_t ownerId;
    uint32_t creationTick;
    bool isHoming = false;
};

struct PlayerData
{
    float lastHit = 0;
    int lifes = 10;
    float shotCooldown = 0.0f;
    float homingCooldown = 0.0f;
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

struct DeathEvent {
    glm::vec3 position;
};

class GameWorld {
public:
    auto& GetScene() { return scene_;};
    const auto& GetScene() const { return scene_; };

    void MarkEntityForDelete(uint32_t id);
    RemovedEntities RemoveMarkedEntities();
    void HandleDeaths(RemovedEntities& removed);

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

    std::vector<uint32_t>& GetKilledPlayers() { return killedPlayers_; };
    void ClearKilledPlayers() { killedPlayers_.clear(); };
    std::vector<DeathEvent> DrainDeaths() { return std::exchange(pendingDeaths_, {}); };
    std::vector<DeathEvent> DrainDeathSounds() { return std::exchange(pendingDeathSounds_, {}); };

    uint32_t AddPlayer(uint32_t id, PlayerData playerData);
    uint32_t AddNpc(uint32_t id);
    uint32_t AddShot(uint32_t id, uint32_t shooterId, bool predicted = false, uint32_t creationTick = 0, bool isHoming = false);
    void     RemoveEntity(uint32_t id);

    void ReassignShotId(uint32_t oldId, uint32_t newId);

    bool IsPlayer(uint32_t id) const { return playerData_.contains(id); };
    bool IsNpc(uint32_t id) const { return npcData_.contains(id); };
    bool IsShot(uint32_t id) const { return shotData_.contains(id); };
private:

    Scene scene_;
    std::unordered_map<uint32_t, NpcData> npcData_;
    std::unordered_map<uint32_t, ShotData> shotData_;
    std::unordered_map<uint32_t, PlayerData> playerData_;

    std::vector<uint32_t> killedPlayers_;
    std::vector<DeathEvent> pendingDeaths_;
    std::vector<DeathEvent> pendingDeathSounds_;
};

#endif
