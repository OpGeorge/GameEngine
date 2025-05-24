#pragma once

#include "gen_game_object.hpp"
#include "coreV/gen_texture.hpp"
#include <queue>

#define GLM_ENABLE_EXPERIMENTAL

namespace gen {

	class NodePropagationSystem {
	public:
		NodePropagationSystem() = default;
		~NodePropagationSystem() = default;

		NodePropagationSystem(const NodePropagationSystem&) = delete;
		NodePropagationSystem& operator=(const NodePropagationSystem&) = delete;

		// Applies color propagation from a single node
		void propagateFromNode(
			GenGameObject& sourceNode,
			GenGameObject::Map& gameObjects,
			float radius,
			std::shared_ptr<GenTexture> orangeTexture,
			std::shared_ptr<GenTexture> greenTexture,
			int currentFrameIndex,
			std::unordered_map<GenGameObject::id_t, int>& lastChangeTracker);

	private:
		// Helper: get all adjacent nodes within radius
		std::vector<GenGameObject*> findAdjacentNodes(
			GenGameObject& source,
			GenGameObject::Map& gameObjects,
			float maxDistance);
	};

}