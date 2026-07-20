#pragma once

#include <vector>
#include <cstdint>
#include <msgpack.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>

struct InputState {
    uint32_t  id       = 0;
    bool left          = false;
    bool right         = false;
    bool forward       = false;
    bool backward      = false;
    bool shoot         = false;
    bool homingShoot   = false;
    uint32_t tick      = 0;
    float aim_x        = 0.0f;
    float aim_z        = 0.0f;

    MSGPACK_DEFINE(id, left, right, forward, backward, shoot, homingShoot, tick, aim_x, aim_z);
};

struct EntityState {
    uint32_t id;
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;

    MSGPACK_DEFINE(
        id,
        position.x, position.y, position.z,
        scale.x, scale.y, scale.z,
        rotation.x, rotation.y, rotation.z, rotation.w,
        velocity.x, velocity.y, velocity.z,
        angularVelocity.x, angularVelocity.y, angularVelocity.z
    );
};

struct EntityCreationState {
    uint32_t id;
    uint32_t type;
    uint32_t ownerId;
    uint32_t sourceTick;
    float radius;
    bool isHoming = false;

    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;

    MSGPACK_DEFINE(
        id, type, ownerId, sourceTick, radius, isHoming,
        position.x, position.y, position.z,
        scale.x, scale.y, scale.z,
        rotation.x, rotation.y, rotation.z, rotation.w,
        velocity.x, velocity.y, velocity.z,
        angularVelocity.x, angularVelocity.y, angularVelocity.z
    );
};

struct PlayerDataState {
    uint32_t id;
    float lastHit;
    int lifes;

    MSGPACK_DEFINE(
        id, lastHit, lifes
    );
};

struct NpcDataState {
    uint32_t id;
    float lastHit;
    int lifes;

    MSGPACK_DEFINE(
        id, lastHit, lifes
    );
};

struct GameState {
    uint32_t tick = 0;
    std::unordered_map<uint32_t, std::pair<uint32_t, uint8_t>> playerToLastProcessedInputAndQueueDepth;
    std::vector<EntityState> entities;
    std::vector<uint32_t> destroyedEntities;
    std::vector<EntityCreationState> createdEntities;
    std::vector<PlayerDataState> playerData;
    std::vector<NpcDataState> npcData;

    bool eventsApplied = false;

    MSGPACK_DEFINE(tick, playerToLastProcessedInputAndQueueDepth, entities, destroyedEntities, createdEntities, playerData, npcData);
};
