#ifndef SCENE_H
#define SCENE_H

#include "model.h"

struct ModelWithReference
{
    unsigned int Id;
    Model model;

    ModelWithReference(unsigned int id, Model&& m)
        : Id(id), model(std::move(m)) {}
};

struct ShotData
{

};

struct PlayerData
{
    uint32_t id;
    float lastHit = 0;
    int lifes = 10;
};

class Scene
{
public:
    Scene();

    bool ModelExists(unsigned int id);
    unsigned int AddModel(Model& model, int id = -1);
    const std::unordered_map<uint32_t, Model>& GetModels() const;
    const std::unordered_map<uint32_t, std::reference_wrapper<const Model>> GetPlayerModels() const;
    Model& GetModelByReference(unsigned int id);
    const Model& GetModelByReference(unsigned int id) const;
    void RemoveModel(unsigned int id);

    void MarkModelForDelete(unsigned int id);
    void RemoveMarkedModels();
    const std::vector<unsigned int>& GetRemoveMarkedModels() const;

    const std::vector<unsigned int>& GetAddedModels() const;
    void ClearAddedModels();
    void AddExtraModelToAddedIds(unsigned int id);
        
    PlayerData& GetPlayerData(uint32_t id);
    std::unordered_map<uint32_t, PlayerData>& GetPlayerData();
    const std::unordered_map<uint32_t, PlayerData>& GetPlayerData() const;
    void AddOrUpdatePlayerData(PlayerData pd);
    void RemovePlayerData(uint32_t id);

    uint32_t currentTick = 0;

    glm::vec3 currentFurthestPosition_;

private:
    unsigned int nextId_;
    std::unordered_map<uint32_t, Model> models_;

    std::unordered_map<uint32_t, ShotData> shotData_;
    std::unordered_map<uint32_t, PlayerData> playerData_;

    std::vector<unsigned int> removeMarkedModels_;
    std::vector<unsigned int> addedModels_;
};

#endif
