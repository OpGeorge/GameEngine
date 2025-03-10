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
		createPipeline();

	
	}

	AppCtrl::~AppCtrl() {
		vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);

	}

	void AppCtrl::run() {
		
		while (!genWindow.shouldClose()) {
			
			glfwPollEvents();
			if (auto commandBuffer = genRenderer.beginFrame()) {

				genRenderer.beginSwachChainRenderPass(commandBuffer);
				renderGameObjcets(commandBuffer);
				genRenderer.endSwachChainRenderPass(commandBuffer);
				genRenderer.endFrame();
			}

		
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
		
	
		assert(pipelineLayout != nullptr && "\nCannot create pipeline before pipeline layout \n\n\n");



		PipelineConfigInfo pipelineConfig{};
		GenPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = genRenderer.getSwapChainRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		genPipeline = std::make_unique<GenPipeline>(genDevice, "shaders/simple_shader.vert.spv",
				"shaders/simple_shader.frag.spv", pipelineConfig);

	
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

	
}