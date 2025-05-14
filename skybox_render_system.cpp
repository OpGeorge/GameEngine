#include "skybox_render_system.hpp"
#include <stdexcept>
#include <array>

namespace gen {

    SkyboxRenderSystem::SkyboxRenderSystem(
        GenDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout skyboxSetLayout)
        : genDevice(device) {

        createPipelineLayout(globalSetLayout, skyboxSetLayout);
        createPipeline(renderPass);
    }

    SkyboxRenderSystem::~SkyboxRenderSystem() {
        vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);
    }

    void SkyboxRenderSystem::createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout,
        VkDescriptorSetLayout skyboxSetLayout) {

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
            globalSetLayout,  // set 0: camera UBO (already exists)
            skyboxSetLayout   // set 1: cubemap sampler
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(genDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create skybox pipeline layout");
        }
    }

    void SkyboxRenderSystem::createPipeline(VkRenderPass renderPass) {
        PipelineConfigInfo pipelineConfig{};
        GenPipeline::defaultPipelineConfigInfo(pipelineConfig);

        // Must override AFTER the default function sets its values
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE; // You still want the test
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Or CCW if your cube is wound inside-out

        genPipeline = std::make_unique<GenPipeline>(
            genDevice,
            "shaders/skybox.vert.spv",
            "shaders/skybox.frag.spv",
            pipelineConfig);
    }

    void SkyboxRenderSystem::render(
        VkCommandBuffer commandBuffer,
        VkDescriptorSet globalDescriptorSet,
        VkDescriptorSet skyboxDescriptorSet) {

        genPipeline->bind(commandBuffer);

        std::array<VkDescriptorSet, 2> sets = {
            globalDescriptorSet, skyboxDescriptorSet
        };

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            static_cast<uint32_t>(sets.size()),
            sets.data(),
            0,
            nullptr);

        vkCmdDraw(commandBuffer, 36, 1, 0, 0);  // No model or vertex buffers needed
    }

}
