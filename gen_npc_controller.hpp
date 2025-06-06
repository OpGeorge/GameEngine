#pragma once
#include "gen_game_object.hpp"
#include "gen_appctrl.hpp"
#include <glm/glm.hpp>

namespace gen {

    class NPCMovementController {
    public:
        void moveToTarget(float dt, GenGameObject& agent, const glm::vec3& targetPosition, float speed = 1.5f);
        GenGameObject* findRandomNearbyWhiteNode(const GenGameObject& npc, GenGameObject::Map& gameObjects, float radius);
        void updateAI(float dt, GenGameObject& npc, GenGameObject::Map& gameObjects, std::shared_ptr<GenTexture> whiteTexture, AppCtrl* appCtrl);
        GenGameObject* findHighestPriorityNearbyNode(const GenGameObject& npc, GenGameObject::Map& gameObjects, float radius);
    };

}