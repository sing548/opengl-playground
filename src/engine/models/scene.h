#ifndef SCENE_H
#define SCENE_H

#include "model.h"

class Scene
{
public:
    Scene();

    bool ModelExists(unsigned int id);
    unsigned int AddModel(Model& model, int id = -1);
    const std::unordered_map<uint32_t, Model>& GetModels() const;
    Model& GetModelByReference(unsigned int id);
    const Model& GetModelByReference(unsigned int id) const;
    void RemoveModel(unsigned int id);

    void MarkModelForDelete(unsigned int id);
    void RemoveMarkedModels();
    const std::vector<unsigned int>& GetRemoveMarkedModels() const;

    const std::vector<unsigned int>& GetAddedModels() const;
    void ClearAddedModels();
    void AddExtraModelToAddedIds(unsigned int id);

    uint32_t currentTick = 0;

    glm::vec3 currentFurthestPosition_;

private:
    unsigned int nextId_;
    std::unordered_map<uint32_t, Model> models_;

    std::vector<unsigned int> removeMarkedModels_;
    std::vector<unsigned int> addedModels_;
};

#endif
