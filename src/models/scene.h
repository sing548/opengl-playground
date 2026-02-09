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
    const std::vector<ModelWithReference>& GetModels() const;
    std::vector<std::reference_wrapper<ModelWithReference>> GetPlayerModels();
    Model& GetModelByReference(unsigned int id);
    void RemoveModel(unsigned int Id);

    uint32_t currentTick = 0;

private:
    unsigned int nextId_;
    std::vector<ModelWithReference> models_;
};

#endif
