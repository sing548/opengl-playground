#include "scene.h"

Scene::Scene() : nextId_(1) {}

unsigned int Scene::AddModel(Model model)
{
    unsigned int id = nextId_++;
    models_.emplace_back(id, std::move(model));
    return id;
}

void Scene::RemoveModel(unsigned int id)
{
    auto it = std::remove_if(models_.begin(), models_.end(),
                             [id](const ModelWithReference& mw) {
                                 return mw.Id == id;
                             });
    models_.erase(it, models_.end());
}

const std::vector<ModelWithReference>& Scene::GetModels() const
{
    return models_;
}