#include "gen_appctrl.hpp"

#include <stdexcept>
#include <array>


#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

namespace gen {

	struct SimplePushConstantData {
		glm::mat2 transform{1.f};
		glm::vec2 offfset;
		alignas(16) glm::vec3 color;

	};


	AppCtrl::AppCtrl() {
		loadGameObjects();
		createPipelineLayot();
		recreateSwapChain();
		createCommandBuffers();

	
	}

	AppCtrl::~AppCtrl() {
		vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);

	}

	void AppCtrl::run() {
		
		while (!genWindow.shouldClose()) {
			
			glfwPollEvents();
			drawFrame();

		
		}
		vkDeviceWaitIdle(genDevice.device());
	}

	void AppCtrl::loadGameObjects() {
	
		std::vector<GenModel::Vertex> vertices{
			{{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
			{{0.5f,0.5f},{0.0f,1.0f,0.0f}},
			{{-0.5f,0.5f},{0.0f,0.0f,1.0f}},
		};

		auto genModel = std::make_shared<GenModel>(genDevice, vertices);
		auto triangle = GenGameObject::createGameObject();
		
		triangle.model = genModel;
		triangle.color = { 0.1f,0.8f,0.1f };
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f,.5f };
		triangle.transform2d.rotation = -.25f * glm::two_pi<float>();


		gameObjects.push_back(std::move(triangle));

	}

	void AppCtrl::createPipelineLayot() {


		VkPushConstantRange pushConstantRange{};

		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


	
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(genDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout ! \n\n\n");

		}


		
	}

	void AppCtrl::createPipeline() {
		
		assert(genSwapChain != nullptr && "\nCannot create pipeline before swap chain! \n\n\n");
		assert(pipelineLayout != nullptr && "\nCannot create pipeline before pipeline layout \n\n\n");



		PipelineConfigInfo pipelineConfig{};
		GenPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = genSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		genPipeline = std::make_unique<GenPipeline>(genDevice, "shaders/simple_shader.vert.spv",
				"shaders/simple_shader.frag.spv", pipelineConfig);

	
	}


	void AppCtrl::recreateSwapChain() {

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
			genSwapChain = std::make_unique<GenSwapChain>(genDevice, extent, std::move(genSwapChain));
			if (genSwapChain->imageCount() != commandBuffers.size()) {
			
				freeCommandBuffers();
				createCommandBuffers();
			
			}
		}
		createPipeline();
	}
	


	void AppCtrl::createCommandBuffers() {

		commandBuffers.resize(genSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = genDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(genDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers! \n\n\n");
		}

	}

	void AppCtrl::freeCommandBuffers() {
		vkFreeCommandBuffers(genDevice.device(), genDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		commandBuffers.clear();
	}

	void AppCtrl::recordCommandBuffer(int imageIndex) {
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer! \n\n\n");

		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = genSwapChain->getRenderPass();
		renderPassInfo.framebuffer = genSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = genSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.5f,0.5f,0.5f,1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t> (clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(genSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(genSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, genSwapChain -> getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0,1,&viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjcets(commandBuffers[imageIndex]);

		

		vkCmdEndRenderPass(commandBuffers[imageIndex]);

		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer! \n\n\n");

		}

	
	}

	void AppCtrl::renderGameObjcets(VkCommandBuffer commandBuffer) {

		genPipeline->bind(commandBuffer);
	
		for (auto& obj : gameObjects) {
			SimplePushConstantData push{};
			push.offfset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();


			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	
	}

	void AppCtrl::drawFrame() {

		uint32_t imageIndex;
		auto result = genSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;

		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image! \n\n\n");


		}


		recordCommandBuffer(imageIndex);
		result = genSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || genWindow.wasWindowResised()) {

			genWindow.wasWindowResised();
			recreateSwapChain();
			return;

		}

		if (result != VK_SUCCESS) {
			
			throw std::runtime_error("failed to present swap chain image! \n\n\n");

		}

	}
}