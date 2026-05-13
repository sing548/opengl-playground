#ifndef NPC_SYSTEM_H
#define NPC_SYSTEM_H

class GameWorld;
class AssetManager;

class NpcSystem {
public:
    NpcSystem() = default;
    explicit NpcSystem(bool isAuthoritative);

    void Update(float dT, GameWorld& gameWorld, AssetManager& AssMan);
private:
    bool isAuthoritative_;
};

#endif
