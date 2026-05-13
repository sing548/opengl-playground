#include "scene.h"

Scene::Scene() : nextId_(1) {}

bool Scene::ModelExists(unsigned int id)
{
    return models_.find(id) != models_.end();
}

unsigned int Scene::AddModel(Model& model, int id)
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
    
    models_.try_emplace(modelId, model);

    addedModels_.push_back(modelId);

    return modelId;
}

void Scene::AddExtraModelToAddedIds(unsigned int id)
{
    addedModels_.push_back(id);
}

void Scene::RemoveModel(unsigned int id)
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

Model& Scene::GetModelByReference(unsigned int id) 
{
    return models_.at(id);
}

const Model& Scene::GetModelByReference(unsigned int id) const
{
    return models_.at(id);
}

void Scene::MarkModelForDelete(unsigned int id)
{
    removeMarkedModels_.push_back(id);
}

void Scene::RemoveMarkedModels()
{
    for (unsigned int id : removeMarkedModels_)
        RemoveModel(id);
    
    removeMarkedModels_.clear();
}

const std::vector<unsigned int>& Scene::GetRemoveMarkedModels() const 
{
    return removeMarkedModels_;
}

const std::vector<unsigned int>& Scene::GetAddedModels() const
{
    return addedModels_;
}

void Scene::ClearAddedModels()
{
    addedModels_.clear();
}
