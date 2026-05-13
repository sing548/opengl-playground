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

class Scene
{
public:
    Scene();

    bool ModelExists(unsigned int id);
    unsigned int AddModel(Model& model, int id = -1);
    const std::unordered_map<uint32_t, Model>& GetModels() const;
    const std::unordered_map<uint32_t, std::reference_wrapper<const Model>> GetNPCModels() const;
    const std::unordered_map<uint32_t, std::reference_wrapper<const Model>> GetPlayerModels() const;
    const std::unordered_map<uint32_t, std::reference_wrapper<const Model>> GetPhysicalModels() const;
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
