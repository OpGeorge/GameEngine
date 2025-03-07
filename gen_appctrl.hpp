#pragma once

#include "gen_window.hpp"
#include "gen_pipeline.hpp"
#include "gen_device.hpp"
#include "gen_swap_chain.hpp"

#include "gen_game_object.hpp"


#include <memory>
#include <vector>


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

	private:

		void loadGameObjects();
		void createPipelineLayot();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjcets(VkCommandBuffer commandBuffer);




		GenWindow genWindow{ WIDTH,HEIGHT,"HELLO VULKAN ENGINE!" };

		GenDevice genDevice{ genWindow };

		std::unique_ptr<GenSwapChain> genSwapChain;

		std::unique_ptr<GenPipeline> genPipeline;

		VkPipelineLayout pipelineLayout;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<GenGameObject> gameObjects;


	};
}