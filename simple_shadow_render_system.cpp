#include "simple_shadow_render_system.hpp"
#include "gen_model.hpp"

#include <stdexcept>
#include <glm/glm.hpp>
#include <iostream>

namespace gen {

    struct PushConstantData {
        glm::mat4 modelMatrix{ 1.0f };
        glm::mat4 lightSpaceMatrix{ 1.0f };
    };

    SimpleShadowRenderSystem::SimpleShadowRenderSystem(GenDevice& device, VkRenderPass shadowRenderPass)
        : genDevice{ device } {
        createPipelineLayout();
        createPipeline(shadowRenderPass);
    }

    void SimpleShadowRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(genDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void SimpleShadowRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GenPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        // Depth only
        pipelineConfig.colorBlendAttachment.blendEnable = VK_FALSE;
        pipelineConfig.hasColorAttachment = false;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;

        pipeline = std::make_unique<GenPipeline>(
            genDevice,
            "shaders/shadow_map.vert.spv",
            "shaders/shadow_map.frag.spv",
            pipelineConfig
        );
    }

    void SimpleShadowRenderSystem::renderObjects(
        VkCommandBuffer commandBuffer,
        GenPointShadowMap& shadowMap,
        const GenGameObject::Map& objects
    ) {
        pipeline->bind(commandBuffer);

        for (int face = 0; face < 6; ++face) {
            shadowMap.beginFaceRenderPass(commandBuffer, face);

            
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(GenPointShadowMap::SHADOW_MAP_RESOLUTION);
            viewport.height = static_cast<float>(GenPointShadowMap::SHADOW_MAP_RESOLUTION);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = { GenPointShadowMap::SHADOW_MAP_RESOLUTION, GenPointShadowMap::SHADOW_MAP_RESOLUTION };
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            for (const auto& [id, obj] : objects) {
                if (!obj.model) continue;

                PushConstantData push{};
                push.modelMatrix = obj.transform.mat4();
                push.lightSpaceMatrix = shadowMap.getLightSpaceMatrix(face);

                vkCmdPushConstants(
                    commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof(PushConstantData),
                    &push
                );

                obj.model->bind(commandBuffer);
                obj.model->draw(commandBuffer);
                
            }

            shadowMap.endRenderPass(commandBuffer);
        }
    }

}
