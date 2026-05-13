#include "game-world.h"

void GameWorld::AddOrUpdatePlayerData(PlayerData pd)
{
    playerData_[pd.id] = std::move(pd);
}

void GameWorld::RemovePlayerData(uint32_t id)
{
    auto it = playerData_.find(id);
    if (it != playerData_.end())
    {
        playerData_.erase(it);
    }
}

// ------------ ToDo: Implement before further implementation of systems

uint32_t AddPlayer(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
    return id;
}

uint32_t AddNPC(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
    return id;
}

uint32_t AddShot(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
    return id;
}

void RemoceEntity(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
}
