#include "scene.h"

Scene::Scene() : nextId_(1) {}

bool Scene::ModelExists(unsigned int id)
{
    for (const auto& mw : models_)
    {
        if (mw.Id == id)
            return true;
    }
    return false;
}

unsigned int Scene::AddModel(Model& model, int id)
{
    unsigned int modelId;

    if (id < 0)
    {
        modelId = nextId_++;
    }
    else 
    {
        modelId = id;
        nextId_ = id + 1;     
    }
    
    models_.emplace_back(modelId, std::move(model));
    return modelId;
}

unsigned int Scene::AddModelWithId(Model& model, unsigned int id)
{
    models_.emplace_back(id, std::move(model));
    nextId_ = id++;
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

Model& Scene::GetModelByReference(unsigned int id) {
    for (auto& mw : models_) {
        if (mw.Id == id) {
            return mw.model;
        }
    }
    throw std::out_of_range("No model found with given ID");
}

std::vector<std::reference_wrapper<ModelWithReference>> Scene::GetPlayerModels()
{
    std::vector<std::reference_wrapper<ModelWithReference>> result;
    result.reserve(models_.size());

    std::copy_if(
        models_.begin(), models_.end(),
        std::back_inserter(result),
        [](ModelWithReference& m){ return m.model.type_ == ModelType::PLAYER; }
    );

    return result;
}
