#include "gen_npc_controller.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp> 
#include <iostream>

namespace gen {

    void NPCMovementController::moveToTarget(float dt, GenGameObject& agent, const glm::vec3& targetPosition, float speed) {
        glm::vec2 currentPosXZ = { agent.transform.translation.x, agent.transform.translation.z };
        glm::vec2 targetPosXZ = { targetPosition.x, targetPosition.z };

        glm::vec2 direction = targetPosXZ - currentPosXZ;

        float distanceSquared = glm::length2(direction);
        if (distanceSquared < 0.0001f) {
            return; // at destination
        }

        glm::vec2 movementDir = glm::normalize(direction);
        float distanceToMove = speed * dt;

        if (glm::length(direction) <= distanceToMove) {
            // snap npc to the position
            agent.transform.translation.x = targetPosition.x;
            agent.transform.translation.z = targetPosition.z;
        }
        else {
            // movement
            agent.transform.translation.x += movementDir.x * distanceToMove;
            agent.transform.translation.z += movementDir.y * distanceToMove;
        }

    }
}