#pragma once

#include "gen_device.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace gen {

    class GenShadowMap {
    public:
        static constexpr uint32_t SHADOW_MAP_WIDTH = 2048;
        static constexpr uint32_t SHADOW_MAP_HEIGHT = 2048;

        GenShadowMap(GenDevice& device);
        ~GenShadowMap();

        GenShadowMap(const GenShadowMap&) = delete;
        GenShadowMap& operator=(const GenShadowMap&) = delete;

        void beginShadowPass(VkCommandBuffer commandBuffer);
        void endShadowPass(VkCommandBuffer commandBuffer);

        VkRenderPass getRenderPass() const { return shadowRenderPass; }
        VkImageView getDepthImageView() const { return depthImageView; }
        VkSampler getShadowSampler() const { return shadowSampler; }
        VkDescriptorImageInfo descriptorInfo() const;

        glm::mat4 getLightViewProjMatrix() const;

    private:
        void createShadowResources();
        void createRenderPass();
        void createFramebuffer();

        GenDevice& genDevice;

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        VkSampler shadowSampler;

        VkRenderPass shadowRenderPass;
        VkFramebuffer shadowFramebuffer;

        glm::mat4 lightViewProj;
    };
}
