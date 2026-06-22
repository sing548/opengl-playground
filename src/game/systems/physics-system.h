#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../../engine/models/scene.h"
#include "../../engine/systems/i-gameplay-system.h"

class GameWorld;
class ITerrainHandler;
struct SystemsContext;

class PhysicsSystem : public IGameplaySystem
{
public:
    void Update(SystemsContext& ctx) override;
    GameplayPhase GetPhase() const override { return GameplayPhase::Simulation; }
    bool CanReplay() override { return true; }
    int GetOrder() const override { return 0; }
private:
    void MoveModels(float dT, GameWorld& gameWorld, bool authoritative);
    void MoveModel(float dT, Scene& Scene, unsigned int id, const glm::vec3& change);
    void CheckHits(GameWorld& gameWorld, ITerrainHandler& terrain, bool authoritative, bool predictive);
    bool Collide(const Model& a, const Model& b);
    void TryCollide(Scene& scene, uint32_t idA, uint32_t idB);
    void CorrectZOffset(Scene& scene);
    bool CollideTerrain(ITerrainHandler& terrain, Scene& scene, uint32_t id, bool fragile = false);
};

#endif
