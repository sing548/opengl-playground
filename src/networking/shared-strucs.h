#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <msgpack.hpp>
#include "../models/scene.h"
#include <glm/vec3.hpp>

struct InputState {
    int  id        = 0;
    bool left      = false;
    bool right     = false;
    bool forward   = false;
    bool backward  = false;
    bool shoot     = false;
    bool shootShot = false;
    uint32_t tick  = 0;

    MSGPACK_DEFINE(id, left, right, forward, backward, shoot, shootShot, tick);
};

struct EntityState {
    uint32_t id;
    glm::vec3 position_;
    glm::vec3 scale_;
    glm::vec3 rotation_;
    glm::vec3 orientation_;
    glm::vec3 baseOrientation_;
    glm::vec3 speed_;
    glm::vec3 rotationSpeed_;

    MSGPACK_DEFINE(
        id,
        position_.x, position_.y, position_.z,
        scale_.x, scale_.y, scale_.z,
        rotation_.x, rotation_.y, rotation_.z,
        orientation_.x, orientation_.y, orientation_.z,
        baseOrientation_.x, baseOrientation_.y, baseOrientation_.z,
        speed_.x, speed_.y, speed_.z,
        rotationSpeed_.x, rotationSpeed_.y, rotationSpeed_.z
    );
};

struct EntityCreationState {
    uint32_t id;
    uint32_t type;
    float radius;

    glm::vec3 position_;
    glm::vec3 scale_;
    glm::vec3 rotation_;
    glm::vec3 orientation_;
    glm::vec3 baseOrientation_;
    glm::vec3 speed_;
    glm::vec3 rotationSpeed_;

    MSGPACK_DEFINE(
        id, type, radius, 
        position_.x, position_.y, position_.z,
        scale_.x, scale_.y, scale_.z,
        rotation_.x, rotation_.y, rotation_.z,
        orientation_.x, orientation_.y, orientation_.z,
        baseOrientation_.x, baseOrientation_.y, baseOrientation_.z,
        speed_.x, speed_.y, speed_.z,
        rotationSpeed_.x, rotationSpeed_.y, rotationSpeed_.z
    );
};

struct GameState {
    uint32_t tick = 0;
    unsigned int playerId = 0;
    std::vector<EntityState> entities;
    std::vector<uint32_t> destroyedEntities;
    std::vector<EntityCreationState> createdEntities;

    MSGPACK_DEFINE(tick, playerId, entities, destroyedEntities, createdEntities);
};
