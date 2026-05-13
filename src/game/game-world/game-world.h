#include <unordered_map>
#include <cstdint>

#include "../../engine/models/scene.h"

struct ShotData
{

};

struct PlayerData
{
    uint32_t id;
    float lastHit = 0;
    int lifes = 10;
};

struct NpcData
{
    
};

class GameWorld {
public:
    Scene& GetScene();
    const Scene& GetScene() const;

    PlayerData& GetPlayerData(uint32_t id);
    std::unordered_map<uint32_t, PlayerData>& GetPlayerData();
    const std::unordered_map<uint32_t, PlayerData>& GetPlayerData() const;
    void AddOrUpdatePlayerData(PlayerData pd);
    void RemovePlayerData(uint32_t id);
private:
    Scene scene_;
    std::unordered_map<uint32_t, NpcData> npcData_;
    std::unordered_map<uint32_t, ShotData> shotData_;
    std::unordered_map<uint32_t, PlayerData> playerData_;
};
