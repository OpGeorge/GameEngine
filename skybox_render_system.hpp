#pragma once

#include "gen_camera.hpp"
#include "gen_device.hpp"
#include "gen_pipeline.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace gen {

    class SkyboxRenderSystem {
    public:
        SkyboxRenderSystem(GenDevice& device, VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout skyboxSetLayout);
        ~SkyboxRenderSystem();

        SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
        SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

        void render(VkCommandBuffer commandBuffer,
            VkDescriptorSet globalDescriptorSet,
            VkDescriptorSet skyboxDescriptorSet);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout,
            VkDescriptorSetLayout skyboxSetLayout);
        void createPipeline(VkRenderPass renderPass);

        GenDevice& genDevice;
        std::unique_ptr<GenPipeline> genPipeline;
        VkPipelineLayout pipelineLayout;
    };

}
