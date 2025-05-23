#pragma once

#include "coreV/gen_device.hpp"
#include "gen_game_object.hpp"

#include "coreV/gen_renderer.hpp"
#include "coreV/gen_window.hpp"

#include "coreV/gen_descriptors.hpp"
#include "gen_logic_manager.hpp"

#include "node_propagation_system.hpp"

#include <memory>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

namespace gen {


	class AppCtrl {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		AppCtrl();
		~AppCtrl();

		AppCtrl(const AppCtrl&) = delete;
		AppCtrl &operator=(const AppCtrl&) = delete;

		void run();

		void refreshObjectDescriptorsIfNeeded(std::shared_ptr<GenTexture> fallbackTexture,
			std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
			std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
			std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets,
			GenDescriptorSetLayout& globalSetLayout,
			GenDescriptorPool& globalPool);

		std::shared_ptr<GenTexture> getCachedTexture(const std::string& path);
		void updateNodeColorAndTextureFromPlayer(GenGameObject& player,
			GenGameObject::Map& gameObjects,
			const std::shared_ptr<GenTexture>& redTexture,
			const std::shared_ptr<GenTexture>& orangeTexture,
			const std::shared_ptr<GenTexture>& greenTexture, 
			int currentFrameNumber,
			std::queue<std::tuple<int, GenGameObject*, std::shared_ptr<GenTexture>, std::shared_ptr<GenTexture>>>& pendingTextureSwaps);

	private:

		void loadGameObjects();
	
		GenWindow genWindow{ WIDTH,HEIGHT,"HELLO VULKAN ENGINE!" };

		GenDevice genDevice{ genWindow };

		GenRenderer genRenderer{ genWindow,genDevice };

		std::unique_ptr<GenDescriptorPool> globalPool{};

		GenGameObject::Map gameObjects;

		GenLogicManager logicManager;

		std::unordered_map<std::string, std::shared_ptr<GenTexture>> textureCache;

		NodePropagationSystem propagationSystem;

		glm::vec3 lastPlayerPosition = {};

	};
}