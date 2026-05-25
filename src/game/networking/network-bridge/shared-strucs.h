#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <msgpack.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/quaternion_float.hpp>

struct InputState {
    int  id        = 0;
    bool left      = false;
    bool right     = false;
    bool forward   = false;
    bool backward  = false;
    bool shoot     = false;
    uint32_t tick  = 0;

    MSGPACK_DEFINE(id, left, right, forward, backward, shoot, tick);
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
    float radius;

    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;

    MSGPACK_DEFINE(
        id, type, radius, 
        position.x, position.y, position.z,
        scale.x, scale.y, scale.z,
        rotation.x, rotation.y, rotation.z, rotation.w,
        velocity.x, velocity.y, velocity.z,
        angularVelocity.x, angularVelocity.y, angularVelocity.z
    );
};

struct GameState {
    uint32_t tick = 0;
    std::vector<EntityState> entities;
    std::vector<uint32_t> destroyedEntities;
    std::vector<EntityCreationState> createdEntities;

    MSGPACK_DEFINE(tick, entities, destroyedEntities, createdEntities);
};
