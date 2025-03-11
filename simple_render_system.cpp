#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>


#define GLM_FORCE_RADIENTS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

namespace gen {

	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		
		alignas(16) glm::vec3 color;
		

	};


	SimpleRenderSystem::SimpleRenderSystem(GenDevice& device, VkRenderPass renderPass) : genDevice{device} {
		
		createPipelineLayot();
		createPipeline(renderPass);


	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);

	}



	void SimpleRenderSystem::createPipelineLayot() {


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

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {


		assert(pipelineLayout != nullptr && "\nCannot create pipeline before pipeline layout \n\n\n");



		PipelineConfigInfo pipelineConfig{};
		GenPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		genPipeline = std::make_unique<GenPipeline>(genDevice, "shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv", pipelineConfig);


	}


	void SimpleRenderSystem::renderGameObjcets(VkCommandBuffer commandBuffer, std::vector<GenGameObject>& gameObjects, const GenCamera& camera) {

		genPipeline->bind(commandBuffer);

		auto projection = camera.getProjcetion() * camera.getView();

		for (auto& obj : gameObjects) {
			obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + .001f, glm::two_pi<float>());
			obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + .0005f, glm::two_pi<float>());

			SimplePushConstantData push{};
			
			push.color = obj.color;
			push.transform = projection * obj.transform.mat4();


			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}

	}


}