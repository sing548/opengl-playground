#ifndef TERRAIN_SYSTEM_H
#define TERRAIN_SYSYTEM_H

#include "../rendering/terrain/terrain-handler.h"

struct SystemsContext;

class TerrainSystem
{
public:
    void Update(SystemsContext& ctx);
private:
    glm::ivec2 lastArea_ { INT_MAX, INT_MAX };
};

#endif
