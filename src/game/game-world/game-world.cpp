#include "game-world.h"

void GameWorld::AddOrUpdatePlayerData(PlayerData pd)
{
    playerData_[pd.id] = std::move(pd);
}


void GameWorld::MarkEntityForDelete(uint32_t id)
{
    if (!scene_.ModelExists(id)) return;
    scene_.MarkModelForDelete(id);
}

std::vector<uint32_t> GameWorld::RemoveMarkedEntities()
{
    std::vector<uint32_t> removed;
    for (uint32_t id : scene_.GetRemoveMarkedModels())
    {
        npcData_.erase(id);
        shotData_.erase(id);
        playerData_.erase(id);
        removed.push_back(id);
    }
    scene_.RemoveMarkedModels();

    return removed;
}

// ------------ ToDo: Implement before further implementation of systems

uint32_t GameWorld::AddPlayer(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
    return id;
}

uint32_t GameWorld::AddNPC(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
    return id;
}

uint32_t GameWorld::AddShot(uint32_t id)
{
    shotData_[id] = ShotData();
    return id;
}

void GameWorld::RemoveEntity(uint32_t id)
{
    throw std::runtime_error("Not Implemented");
}
