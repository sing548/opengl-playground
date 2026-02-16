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
    unsigned int AddModelWithId(Model& model, unsigned int id);
    const std::unordered_map<uint32_t, Model>& GetModels() const;
    const std::unordered_map<uint32_t, std::reference_wrapper<const Model>> GetPlayerModels() const;
    Model& GetModelByReference(unsigned int id);
    void RemoveModel(unsigned int Id);

    uint32_t currentTick = 0;

private:
    unsigned int nextId_;
    std::unordered_map<uint32_t, Model> models_;
};

#endif
