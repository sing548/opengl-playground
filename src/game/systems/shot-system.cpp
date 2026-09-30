#include "shot-system.h"

#include "../../engine/systems/system-structs.h"

#include "../game-world/game-world.h"

void ShotSystem::Update(SystemsContext& ctx)
{
    for (auto& [id, data] : ctx.world.GetShotData())
    {
        data.age += ctx.dT;

        if (ctx.authoritative && data.age > SHOT_LIFETIME)
            ctx.world.MarkEntityForDelete(id);
    }
}
