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
		static constexpr int WIDTH = 1280;
		static constexpr int HEIGHT = 720;

		AppCtrl();
		~AppCtrl();

		AppCtrl(const AppCtrl&) = delete;
		AppCtrl &operator=(const AppCtrl&) = delete;

		void run();

		void refreshObjectDescriptorsIfNeeded(
	
			std::shared_ptr<GenTexture> fallbackTexture,
			std::vector<std::unique_ptr<GenBuffer>>& uboBuffers,
			std::unordered_map<GenGameObject::id_t, std::vector<std::unique_ptr<GenBuffer>>>& textureToggleBuffers,
			std::array<std::unordered_map<GenGameObject::id_t, VkDescriptorSet>, GenSwapChain::MAX_FRAMES_IN_FLIGHT>& objectDescriptorSets,
			GenDescriptorSetLayout& globalSetLayout,
			GenDescriptorPool& globalPool
		);

		std::shared_ptr<GenTexture> getCachedTexture(const std::string& path);
		void updateNodeColorAndTextureFromPlayer(GenGameObject& player,
			GenGameObject::Map& gameObjects,
			const std::shared_ptr<GenTexture>& redTexture,
			const std::shared_ptr<GenTexture>& orangeTexture,
			const std::shared_ptr<GenTexture>& greenTexture,
			int currentFrameIndex,
			float elapsedTime
			);

		void resetLevelTransforms(const std::unordered_map<std::string, TransformComponent>& initialTransforms);
		int currentLevel;

		std::unordered_map<std::string, TransformComponent> initialTransformsLevel1;
		std::unordered_map<std::string, TransformComponent> initialTransformsLevel2;

	private:

		void loadGameObjects();
	
		GenWindow genWindow{ WIDTH,HEIGHT,"HELLO VULKAN ENGINE!" };

		GenDevice genDevice{ genWindow };

		GenRenderer genRenderer{ genWindow,genDevice };

		std::unique_ptr<GenDescriptorPool> globalPool{};

		GenGameObject::Map gameObjects;

		

		GenGameObject::Map* activeGameObjects = nullptr;

		GenLogicManager logicManager;

		std::unordered_map<std::string, std::shared_ptr<GenTexture>> textureCache;

		NodePropagationSystem propagationSystem;

		glm::vec3 lastPlayerPosition = {};

		std::unordered_map<GenGameObject::id_t, int> lastTextureChangeFrame;

		std::unordered_map<GenGameObject::id_t, float> nodeCooldowns;



	};
}