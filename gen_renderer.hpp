
#pragma once

#include "gen_window.hpp"
#include "gen_device.hpp"
#include "gen_swap_chain.hpp"





#include <memory>
#include <vector>
#include <cassert>

namespace gen {


	class GenRenderer {

	public:
		

		GenRenderer(GenWindow &window, GenDevice &device);
		~GenRenderer();

		GenRenderer(const GenRenderer&) = delete;
		GenRenderer& operator=(const GenRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwachChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwachChainRenderPass(VkCommandBuffer commandBuffer);


		bool isFrameInProgress() const { return isFrameStarted; };
		VkRenderPass getSwapChainRenderPass() const { return genSwapChain->getRenderPass(); };
		VkCommandBuffer getCurrentCommandBuffer() const {
			
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress\n\n\n");
			return commandBuffers[currentImageIndex];


		};

	private:

	
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		




		GenWindow& genWindow;

		GenDevice& genDevice;

		std::unique_ptr<GenSwapChain> genSwapChain;

		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;

		bool isFrameStarted{false};
		int currentFrameIndex;

	


	};
}