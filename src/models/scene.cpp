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
    return modelId;
}

unsigned int Scene::AddModelWithId(Model& model, uint32_t id)
{
    models_.try_emplace(id, model);
    return id;
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

Model& Scene::GetModelByReference(unsigned int id) {
    return models_.at(id);
    throw std::out_of_range("No model found with given ID");
}

std::unordered_map<uint32_t, std::reference_wrapper<Model>>Scene::GetPlayerModels()
{
    std::unordered_map<uint32_t, std::reference_wrapper<Model>> result;

    for (auto& [id, modelRef] : models_)
    {
        if (modelRef.type_ == ModelType::PLAYER)
        {
            result.emplace(id, modelRef);
        }
    }

    return result;
}
