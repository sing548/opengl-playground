#ifndef SCENE_H
#define SCENE_H

#include "model.h"

class Scene
{
public:
    Scene();

    bool ModelExists(uint32_t id);
    uint32_t AddModel(Model& model, int id = 0);
    std::unordered_map<uint32_t, Model>& GetModels() { return models_; };
    const std::unordered_map<uint32_t, Model>& GetModels() const;
    Model& GetModelByReference(uint32_t id);
    const Model& GetModelByReference(uint32_t id) const;
    void RemoveModel(uint32_t id);

    void MarkModelForDelete(uint32_t id);
    void RemoveMarkedModels();
    const std::vector<uint32_t>& GetRemoveMarkedModels() const;
    void ClearRemovedModels() { removeMarkedModels_.clear(); };

    const std::vector<uint32_t>& GetAddedModels() const;
    void ClearAddedModels() { addedModels_.clear(); };

    
    void AddExtraModelToAddedIds(uint32_t id);

    uint32_t currentTick = 0;

    glm::vec3 currentFurthestPosition;

private:
    uint32_t nextId_;
    std::unordered_map<uint32_t, Model> models_;

    std::vector<uint32_t> removeMarkedModels_;
    std::vector<uint32_t> addedModels_;
};

#endif
