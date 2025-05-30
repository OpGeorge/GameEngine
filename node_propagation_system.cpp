#include "node_propagation_system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include<iostream>

namespace gen {

	std::vector<GenGameObject*> NodePropagationSystem::findAdjacentNodes(
		GenGameObject& source,
		GenGameObject::Map& gameObjects,
		float maxDistance)
	{
		std::vector<GenGameObject*> result;
		const glm::vec3& sourcePos = source.transform.translation;
		const float maxDistSq = maxDistance * maxDistance;

		for (auto& [id, obj] : gameObjects) {
			if (id == source.getId()) continue;
			if (obj.type != ObjectType::Node || !obj.node) continue;

			if (glm::distance2(sourcePos, obj.transform.translation) <= maxDistSq) {
				result.push_back(&obj);
			}
		}

		return result;
	}

    void NodePropagationSystem::propagateFromNode(
        GenGameObject& sourceNode,
        GenGameObject::Map& gameObjects,
        float radius,
        std::shared_ptr<GenTexture> orangeTexture,
        std::shared_ptr<GenTexture> greenTexture,
        int currentFrameIndex,
        std::unordered_map<GenGameObject::id_t, int>& lastChangeTracker,
        std::unordered_map<GenGameObject::id_t, float>& nodeCooldowns,
        float elapsedTime
        )
    {
        if (!sourceNode.node) return;

        auto sourceColor = sourceNode.node->color;
        auto neighbors = findAdjacentNodes(sourceNode, gameObjects, radius);

       /* std::cout << "[PROPAGATE] Node ID " << sourceNode.getId()
            << ", color: " << static_cast<int>(sourceColor)
            << ", affecting " << neighbors.size() << " neighbors\n";*/

        for (auto* neighbor : neighbors) {
            if (!neighbor->node) continue;
            auto& n = *neighbor->node;

           // std::cout << " Neighbor ID " << neighbor->getId()
             //   << ", current color: " << static_cast<int>(n.color);

            if (sourceColor == NodeComponent::NodeColor::Red) {
                if (n.color == NodeComponent::NodeColor::Red) {
                    //std::cout << " (already Red), skipped\n";
                    continue;
                }

                n.color = NodeComponent::NodeColor::Orange;
                n.parentId = sourceNode.getId();
                n.activated = true;
                n.selfPosition = neighbor->transform.translation;
                nodeCooldowns[neighbor->getId()] = elapsedTime + 7.0;

                if (neighbor->texture != orangeTexture) {
                    const int cooldownFrames = 2;
                    if (lastChangeTracker.find(neighbor->getId()) != lastChangeTracker.end() &&
                        currentFrameIndex - lastChangeTracker[neighbor->getId()] < cooldownFrames) {
                        continue;
                    }

                    neighbor->texture = orangeTexture;
                    neighbor->textureDirty = true;

                    lastChangeTracker[neighbor->getId()] = currentFrameIndex;

                }
                else {
                    //std::cout << " already Orange\n";
                }
            }
            else if (sourceColor == NodeComponent::NodeColor::Orange) {
                if (n.color == NodeComponent::NodeColor::Red ||
                    n.color == NodeComponent::NodeColor::Orange) {
                    //std::cout << " (already Red/Orange), skipped\n";
                    continue;
                }

                n.color = NodeComponent::NodeColor::Green;
                n.parentId = sourceNode.getId();
                n.activated = true;
                n.selfPosition = neighbor->transform.translation;
                nodeCooldowns[neighbor->getId()] = elapsedTime + 7.0;

                if (neighbor->texture != greenTexture) {

                    const int cooldownFrames = 2;
                    if (lastChangeTracker.find(neighbor->getId()) != lastChangeTracker.end() &&
                        currentFrameIndex - lastChangeTracker[neighbor->getId()] < cooldownFrames) {
                        continue;
                    }

                    neighbor->texture = greenTexture;
                    neighbor->textureDirty = true;
                    lastChangeTracker[neighbor->getId()] = currentFrameIndex;
                }
                else {
                    //std::cout << " already Green\n";
                }
            }
            else {
                //std::cout << " (non-propagating color), skipped\n";
            }
        }
    }

}