#pragma once
#include "gen_game_object.hpp"
#include <glm/glm.hpp>

namespace gen {

    class NPCMovementController {
    public:
        void moveToTarget(float dt, GenGameObject& agent, const glm::vec3& targetPosition, float speed = 1.5f);
    };

}