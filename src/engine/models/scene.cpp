#include "scene.h"

Scene::Scene() : nextId_(1) {}

bool Scene::ModelExists(uint32_t id)
{
    return models_.find(id) != models_.end();
}

uint32_t Scene::AddModel(Model& model, int id)
{
    uint32_t modelId;

    if (id > 0)
    {
        modelId = id;
    }
    else 
    {
        modelId = nextId_++;  
    }
    
    auto [it, inserted] = models_.try_emplace(modelId, model);
    if (!inserted) std::cout << "AddModel COLLISION on id " << modelId << "\n";

    addedModels_.push_back(modelId);

    return modelId;
}

void Scene::AddExtraModelToAddedIds(uint32_t id)
{
    addedModels_.push_back(id);
}

void Scene::RemoveModel(uint32_t id)
{
    auto it = models_.find(id);
    if (it != models_.end())
    {
        models_.erase(it);
    }
}

const std::unordered_map<uint32_t, Model>& Scene::GetModels() const
{
    return models_;
}

Model& Scene::GetModelByReference(uint32_t id) 
{
    return models_.at(id);
}

const Model& Scene::GetModelByReference(uint32_t id) const
{
    return models_.at(id);
}

void Scene::ReassignId(uint32_t oldId, uint32_t newId)
{
    auto nh = models_.extract(oldId);
    if (nh.empty()) return;

    nh.key() = newId;
    models_.insert(std::move(nh));
}

void Scene::MarkModelForDelete(uint32_t id)
{
    removeMarkedModels_.push_back(id);
}

void Scene::RemoveMarkedModels()
{
    for (uint32_t id : removeMarkedModels_)
        RemoveModel(id);
    
    ClearRemovedModels();
}

const std::vector<uint32_t>& Scene::GetRemoveMarkedModels() const 
{
    return removeMarkedModels_;
}

const std::vector<uint32_t>& Scene::GetAddedModels() const
{
    return addedModels_;
}
