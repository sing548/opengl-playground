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

    unsigned int AddModel(Model model);
    const std::vector<ModelWithReference>& GetModels() const;
    void RemoveModel(unsigned int Id);

private:
    unsigned int nextId_;
    std::vector<ModelWithReference> models_;
};

#endif
