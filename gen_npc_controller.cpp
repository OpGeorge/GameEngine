#include "gen_npc_controller.hpp"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp> 
#include <iostream>
#include <cstdlib>
#include <ctime>

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

    GenGameObject* NPCMovementController::findRandomNearbyWhiteNode(const GenGameObject& npc, GenGameObject::Map& gameObjects, float radius) {
        glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };
        std::vector<GenGameObject*> nearbyWhites;

        for (auto& [id, obj] : gameObjects) {
            if (obj.type != ObjectType::Node || !obj.node) continue;
            if (obj.node->color != NodeComponent::NodeColor::White) continue;

            glm::vec2 nodePos = { obj.transform.translation.x, obj.transform.translation.z };

            float dist = glm::length(nodePos - npcPos);
            float effectiveRadius = radius;

            if (obj.soundDisc) {
                effectiveRadius += obj.soundDisc->radius;
            }

            if (dist <= effectiveRadius) {
                nearbyWhites.push_back(&obj);
            }
        }

        if (nearbyWhites.empty()) return nullptr;

        int index = rand() % nearbyWhites.size();
        return nearbyWhites[index];
    }

    void NPCMovementController::updateAI(
        float dt,
        GenGameObject& npc,
        GenGameObject::Map& gameObjects,
        std::shared_ptr<GenTexture> whiteTexture,
        AppCtrl* appCtrl
    ) {
        if (!npc.npcBehavior) return;

        const float npcKillRadius = 1.5f;
        glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };

        for (auto& [id, obj] : gameObjects) {
            if (obj.type != ObjectType::Player) continue;

            glm::vec2 playerPos = { obj.transform.translation.x, obj.transform.translation.z };
            float dist = glm::length(npcPos - playerPos);

            if (dist <= npcKillRadius) {
                if (appCtrl->currentLevel % 2 == 0) {
                    std::cout << "[NPC Trigger] Restart due to player proximity\n";
                    appCtrl->resetLevelTransforms(appCtrl->initialTransformsLevel1);
                }
                else {
                    std::cout << "[NPC Trigger] Restart due to player proximity\n";
                    appCtrl->resetLevelTransforms(appCtrl->initialTransformsLevel2);
                }
                  // direct call
                return;
            }
        }

        auto& behavior = *npc.npcBehavior;

        if (behavior.state == NPCState::IdleRoaming) {
            GenGameObject* currentTarget = nullptr;

            GenGameObject* colored = findHighestPriorityNearbyNode(npc, gameObjects, 2.0f);
            if (colored) {
                behavior.state = NPCState::SeekingSoundSource;
                behavior.currentTargetNodeId = colored->getId();
                std::cout << "[NPC] Switching to SEEKING, heading to node " << colored->getId() << "\n";
                return;
            }

            if (behavior.currentTargetNodeId != -1) {
                unsigned int id = static_cast<unsigned int>(behavior.currentTargetNodeId);
                auto it = gameObjects.find(id);
                if (it != gameObjects.end()) {
                    currentTarget = &it->second;
                }
            }

            if (!currentTarget) {
                GenGameObject* next = findRandomNearbyWhiteNode(npc, gameObjects, 2.f);
                if (next) {
                    behavior.currentTargetNodeId = next->getId();
                    currentTarget = next;
                }
                else {
                    return;
                }
            }

            moveToTarget(dt, npc, currentTarget->transform.translation);

            glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };
            glm::vec2 tgtPos = { currentTarget->transform.translation.x, currentTarget->transform.translation.z };
            if (glm::distance2(npcPos, tgtPos) < 0.02f) {
                behavior.currentTargetNodeId = -1;
            }
        }
        else if (behavior.state == NPCState::SeekingSoundSource) {
            GenGameObject* targetNode = nullptr;

            if (behavior.currentTargetNodeId != -1) {
                unsigned int id = static_cast<unsigned int>(behavior.currentTargetNodeId);
                auto it = gameObjects.find(id);
                if (it != gameObjects.end()) {
                    targetNode = &it->second;
                }
            }

            if (!targetNode) {
                behavior.state = NPCState::IdleRoaming;
                behavior.currentTargetNodeId = -1;
                return;
            }

            moveToTarget(dt, npc, targetNode->transform.translation);

            glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };
            glm::vec2 tgtPos = { targetNode->transform.translation.x, targetNode->transform.translation.z };
            if (glm::distance2(npcPos, tgtPos) < 0.02f) {
                auto& node = *targetNode->node;

                node.color = NodeComponent::NodeColor::White;
                node.activated = false;
                node.hasPropagated = false;

                if (targetNode->texture != whiteTexture) {
                    targetNode->texture = whiteTexture;
                    targetNode->textureDirty = true;
                }

                if (node.parentId != -1) {
                    unsigned int parentId = static_cast<unsigned int>(node.parentId);
                    auto pit = gameObjects.find(parentId);
                    if (pit != gameObjects.end()) {
                        behavior.currentTargetNodeId = parentId;
                        behavior.state = NPCState::TracingToRoot;
                        std::cout << "[NPC] Tracing to parent node " << parentId << "\n";
                    }
                    else {
                        behavior.state = NPCState::IdleRoaming;
                        behavior.currentTargetNodeId = -1;
                        std::cout << "[NPC] Reached root node.\n";
                    }
                }
                else {
                    behavior.state = NPCState::IdleRoaming;
                    behavior.currentTargetNodeId = -1;
                    std::cout << "[NPC] Reached root node (no parent).\n";
                }
            }
        }
        else if (behavior.state == NPCState::TracingToRoot) {
            GenGameObject* targetNode = nullptr;

            if (behavior.currentTargetNodeId != -1) {
                unsigned int id = static_cast<unsigned int>(behavior.currentTargetNodeId);
                auto it = gameObjects.find(id);
                if (it != gameObjects.end()) {
                    targetNode = &it->second;
                }
            }

            if (!targetNode) {
                behavior.state = NPCState::IdleRoaming;
                behavior.currentTargetNodeId = -1;
                return;
            }

            moveToTarget(dt, npc, targetNode->transform.translation);

            glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };
            glm::vec2 tgtPos = { targetNode->transform.translation.x, targetNode->transform.translation.z };

            if (glm::distance2(npcPos, tgtPos) < 0.02f) {
                auto& node = *targetNode->node;

                node.color = NodeComponent::NodeColor::White;
                node.activated = false;
                node.hasPropagated = false;

                if (targetNode->texture != whiteTexture) {
                    targetNode->texture = whiteTexture;
                    targetNode->textureDirty = true;
                }

                if (node.parentId != -1) {
                    unsigned int parentId = static_cast<unsigned int>(node.parentId);
                    auto pit = gameObjects.find(parentId);
                    if (pit != gameObjects.end()) {
                        behavior.currentTargetNodeId = parentId;
                        std::cout << "[NPC] Continuing trace to parent node " << parentId << "\n";
                    }
                    else {
                        behavior.state = NPCState::IdleRoaming;
                        behavior.currentTargetNodeId = -1;
                        std::cout << "[NPC] Reached root node.\n";
                    }
                }
                else {
                    behavior.state = NPCState::IdleRoaming;
                    behavior.currentTargetNodeId = -1;
                    std::cout << "[NPC] Reached root node (no parent).\n";
                }
            }
        }

    }


    GenGameObject* NPCMovementController::findHighestPriorityNearbyNode(const GenGameObject& npc, GenGameObject::Map& gameObjects, float radius) {
        glm::vec2 npcPos = { npc.transform.translation.x, npc.transform.translation.z };

        GenGameObject* bestNode = nullptr;
        int bestPriority = -1;  // higher is better

        auto getColorPriority = [](NodeComponent::NodeColor color) {
            switch (color) {
            case NodeComponent::NodeColor::Red: return 3;
            case NodeComponent::NodeColor::Orange: return 2;
            case NodeComponent::NodeColor::Green: return 1;
            default: return 0;  // White or invalid
            }
            };

        for (auto& [id, obj] : gameObjects) {
            if (obj.type != ObjectType::Node || !obj.node) continue;

            auto color = obj.node->color;
            int priority = getColorPriority(color);
            if (priority == 0) continue;

            glm::vec2 nodePos = { obj.transform.translation.x, obj.transform.translation.z };
            float dist = glm::distance(nodePos, npcPos);
            float effectiveRadius = radius;

            if (obj.soundDisc) effectiveRadius += obj.soundDisc->radius;

            if (dist <= effectiveRadius && priority > bestPriority) {
                bestPriority = priority;
                bestNode = &obj;
            }
        }

        return bestNode;
    }

}