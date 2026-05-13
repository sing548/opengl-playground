#include "game-world.h"

void GameWorld::MarkEntityForDelete(uint32_t id)
{
    if (!scene_.ModelExists(id)) return;
    scene_.MarkModelForDelete(id);
}

RemovedEntities GameWorld::RemoveMarkedEntities()
{
    RemovedEntities removed;
    for (uint32_t id : scene_.GetRemoveMarkedModels())
    {
        removed.all.push_back(id);
        if (npcData_.contains(id)) removed.npcs.push_back(id);
        if (shotData_.contains(id)) removed.shots.push_back(id);
        if (playerData_.contains(id)) removed.players.push_back(id);
        npcData_.erase(id);
        shotData_.erase(id);
        playerData_.erase(id);
    }
    scene_.RemoveMarkedModels();

    return removed;
}

// ------------ ToDo: Implement before further implementation of systems

uint32_t GameWorld::AddPlayer(uint32_t id, PlayerData pd)
{
    playerData_[id] = std::move(pd);
    return id;
}

uint32_t GameWorld::AddNpc(uint32_t id)
{
    npcData_[id] = NpcData();
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
