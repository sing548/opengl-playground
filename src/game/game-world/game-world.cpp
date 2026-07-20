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
        //std::cout << "Removed entity: " << id << std::endl;
    }

    HandleDeaths(removed);

    scene_.RemoveMarkedModels();

    return removed;
}

void GameWorld::HandleDeaths(RemovedEntities& removed)
{
    for (uint32_t id : removed.players)
    {
        glm::vec3 pos = scene_.GetModelByReference(id).GetPosition();
        pendingDeaths_.push_back({ pos });
        pendingDeathSounds_.push_back({ pos });
    }

    for (uint32_t id : removed.npcs)
    {
        glm::vec3 pos = scene_.GetModelByReference(id).GetPosition();
        pendingDeaths_.push_back({ pos });
        pendingDeathSounds_.push_back({ pos });
    }
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

uint32_t GameWorld::AddShot(uint32_t id, uint32_t shooterId, bool predicted, uint32_t creationTick, bool isHoming)
{
    shotData_[id] = ShotData { id, shooterId, creationTick, isHoming };
    return id;
}

void GameWorld::RemoveEntity(uint32_t id)
{
    scene_.MarkModelForDelete(id);
    scene_.RemoveModel(id);
}

void GameWorld::ReassignShotId(uint32_t oldId, uint32_t newId)
{
    auto nh = shotData_.extract(oldId);
    if (nh.empty()) return;

    nh.key() = newId;
    shotData_.insert(std::move(nh));
}
