#include "gen_renderer.hpp"

#include <cassert>
#include <stdexcept>
#include <array>


namespace gen {




	GenRenderer::GenRenderer(GenWindow& window, GenDevice& device) : genWindow{ window }, genDevice{device} {
	
		recreateSwapChain();
		createCommandBuffers();


	}

	GenRenderer::~GenRenderer() {
		freeCommandBuffers();

	}

	

	void GenRenderer::recreateSwapChain() {

		auto extent = genWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = genWindow.getExtent();
			glfwWaitEvents();

		}
		vkDeviceWaitIdle(genDevice.device());


		if (genSwapChain == nullptr) {
			genSwapChain = std::make_unique<GenSwapChain>(genDevice, extent);
		}
		else {
			std::shared_ptr<GenSwapChain> oldSwapChain = std::move(genSwapChain);
			genSwapChain = std::make_unique<GenSwapChain>(genDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*genSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed! \n\n\n");


			}

			if (!genSwapChain->imageCount() != commandBuffers.size()) {
			
				freeCommandBuffers();
				createCommandBuffers();
			
			}

		}
		
	}



	void GenRenderer::createCommandBuffers() {

		commandBuffers.resize(GenSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = genDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(genDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers! \n\n\n");
		}

	}

	void GenRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(genDevice.device(), genDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		commandBuffers.clear();
	}

	VkCommandBuffer  GenRenderer::beginFrame() {
		
		assert(!isFrameStarted && "Can't call begin frame while alredy in progress \n\n\n");
		
		auto result = genSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;

		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image! \n\n\n");


		}
		
		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer! \n\n\n");

		}

		return commandBuffer;


	}
	void GenRenderer::endFrame() {
	
		assert(isFrameStarted && "Can not call endframe while frame is not in progress");

		auto commandBuffer = getCurrentCommandBuffer();


		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer! \n\n\n");

		}

		auto result = genSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || genWindow.wasWindowResised()) {

			
			genWindow.resetWindowResisezedFlag();
			recreateSwapChain();
			

		}
		else if (result != VK_SUCCESS) {

			throw std::runtime_error("failed to present swap chain image! \n\n\n");

		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % GenSwapChain::MAX_FRAMES_IN_FLIGHT;
		


	
	}
	void GenRenderer::beginSwachChainRenderPass(VkCommandBuffer commandBuffer) {
	
		assert(isFrameStarted && "Can not call beginSwachChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() &&  "Can not begin render pass on command buffer from a diferent frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = genSwapChain->getRenderPass();
		renderPassInfo.framebuffer = genSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = genSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.5f,0.5f,0.5f,1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t> (clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(genSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(genSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, genSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


	
	}
	void GenRenderer::endSwachChainRenderPass(VkCommandBuffer commandBuffer) {
	
		assert(isFrameStarted && "Can not call endSwachChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can not end render pass on command buffer from a diferent frame");

		vkCmdEndRenderPass(commandBuffer);
		
	
	}


}