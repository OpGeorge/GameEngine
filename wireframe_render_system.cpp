#include "wireframe_render_system.hpp"

#include <stdexcept>

namespace gen {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    WireframeRenderSystem::WireframeRenderSystem(
        GenDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : genDevice{ device } {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    WireframeRenderSystem::~WireframeRenderSystem() {
        vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);
    }

    void WireframeRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &globalSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(genDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create wireframe pipeline layout!");
        }
    }

    void WireframeRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo config{};
        GenPipeline::defaultPipelineConfigInfo(config);

        // Enable transparency (optional, for ghosty wire look)
        GenPipeline::enableAlphaBlending(config);

        // Wireframe mode
        config.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
        config.rasterizationInfo.lineWidth = 1.0f;  // Optional, default is 1.0

        config.renderPass = renderPass;
        config.pipelineLayout = pipelineLayout;

        wirePipeline = std::make_unique<GenPipeline>(
            genDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            config
        );
    }

    void WireframeRenderSystem::render(FrameInfo& frameInfo) {
        wirePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto& [id, obj] : frameInfo.gameObjcets) {
            if (!obj.model) continue;
            if (!obj.soundSphere || !obj.soundSphere->visible) continue;

            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            VkDescriptorSet descriptorSet = frameInfo.objectDescriptorSets.at(id);
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                1,
                &descriptorSet,
                0,
                nullptr);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}
